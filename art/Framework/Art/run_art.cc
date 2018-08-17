#include "art/Framework/Art/run_art.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Framework/Art/detail/info_success.h"
#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/ostream_handle.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/detail/print_mode.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#ifdef __linux__
#include <malloc.h>
#endif // __linux__

using namespace std;
using namespace string_literals;

namespace {

  cet::ostream_handle
  make_ostream_handle(std::string const& filename)
  {
    if (filename.empty()) {
      return cet::ostream_handle{std::cerr};
    } else {
      auto os = cet::ostream_handle{filename};
      if (!os) {
        throw art::Exception{art::errors::Configuration}
          << "Unable to write post-processed configuration to specified file "
          << filename << ".\n";
      }
      return os;
    }
  }

  fhicl::detail::print_mode
  get_print_mode(std::string const& mode)
  {
    if (mode == "raw") {
      return fhicl::detail::print_mode::raw;
    } else if (mode == "annotate") {
      return fhicl::detail::print_mode::annotated;
    } else if (mode == "prefix-annotate") {
      return fhicl::detail::print_mode::prefix_annotated;
    }
    throw art::Exception{art::errors::Configuration}
      << "Unrecognized ParameterSet printing mode: " << mode << '\n';
  }

  std::string
  banner(std::string const& filename)
  {
    std::string result = "** Config output ";
    result +=
      filename.empty() ? "follows" : std::string("to file '" + filename + "'");
    result += " **\n";
    return result;
  }

  enum class debug_processing : std::size_t {
    config_out,
    debug_config,
    validate_config,
    none
  };

  debug_processing
  maybe_output_config(fhicl::ParameterSet const& main_pset,
                      fhicl::ParameterSet const& scheduler_pset)
  {
    if (!scheduler_pset.has_key("debug"))
      return debug_processing::none;

    auto const processing_options = {
      "config-out", "debug-config", "validate-config"};

    auto const& debug_pset = scheduler_pset.get<fhicl::ParameterSet>("debug");

    auto const option = debug_pset.get<std::string>("option");
    auto pos = cet::find_in_all(processing_options, option);
    if (pos == cend(processing_options)) {
      throw art::Exception{
        art::errors::Configuration,
        "An error was encountered while processing debugging options"}
        << "The debugging option '" << option << "' is not supported.\n"
        << "If you did not explicitly provide this value in your configuration "
           "file,"
        << "please contact artists@fnal.gov and report this error.  "
           "Otherwise,\n"
        << "choose from 'configOut', 'debugConfig', or 'validateConfig'.\n";
    }

    auto const index = std::distance(cbegin(processing_options), pos);
    auto const filename = debug_pset.get<std::string>("fileName");
    auto const mode = debug_pset.get<std::string>("printMode");
    std::cerr << banner(filename);
    auto os = make_ostream_handle(filename);
    os << main_pset.to_indented_string(0, get_print_mode(mode));
    return static_cast<debug_processing>(index);
  }

} // unnamed namespace

namespace bpo = boost::program_options;

namespace art {

  int
  run_art(int argc,
          char** argv,
          bpo::options_description& in_desc,
          cet::filepath_maker& lookupPolicy,
          OptionsHandlers&& handlers)
  {
    ostringstream descstr;
    descstr << "\nUsage: "
            << boost::filesystem::path(argv[0]).filename().native()
            << " <-c <config-file>> <other-options> [<source-file>]+\n\n"
            << "Basic options";
    bpo::options_description all_desc{descstr.str()};
    all_desc.add(in_desc);
    // BasicOptionsHandler should always be first in the list!
    handlers.emplace(handlers.begin(),
                     new BasicOptionsHandler{all_desc, lookupPolicy});
    // BasicPostProcessor should be last.
    handlers.emplace_back(new BasicPostProcessor);
    // This must be added separately: how to deal with any non-option arguments.
    bpo::positional_options_description pd;
    // A single non-option argument will be taken to be the source data file.
    pd.add("source", -1);
    // Parse the command line.
    bpo::variables_map vm;
    try {
      bpo::store(bpo::command_line_parser(argc, argv)
                   .options(all_desc)
                   .positional(pd)
                   .run(),
                 vm);
      bpo::notify(vm);
    }
    catch (bpo::error const& e) {
      cerr << "Exception from command line processing in " << argv[0] << ": "
           << e.what() << "\n";
      return 88;
    }
    // Preliminary argument checking.
    for (auto& handler : handlers) {
      auto result = handler->checkOptions(vm);
      if (result != 0) {
        return result;
      }
    }
    // Processing of arguments and post-processing of config.
    fhicl::intermediate_table raw_config;
    for (auto& handler : handlers) {
      auto result = handler->processOptions(vm, raw_config);
      if (result != 0) {
        return result;
      }
    }

    // If configuration pruning has been enabled, remove unused module
    // configurations.
    using detail::exists_outside_prolog;
    assert(exists_outside_prolog(raw_config, "services.scheduler"));
    try {
      auto const result = detail::detect_unused_configuration(raw_config);
      std::string const pruneConfig{"services.scheduler.pruneConfig"};
      if (exists_outside_prolog(raw_config, pruneConfig) &&
          raw_config.get<bool>(pruneConfig)) {
        detail::prune_configuration(result.first, result.second, raw_config);
      }
    }
    catch (Exception const& e) {
      printArtException(e, "art");
      return e.returnCode();
    }
    catch (cet::exception const& e) {
      printArtException(e, "art");
      return 65;
    }
    catch (std::bad_alloc const& bda) {
      printBadAllocException("art");
      return 68;
    }
    catch (std::exception const& e) {
      printStdException(e, "art");
      return 66;
    }
    catch (...) {
      printUnknownException("art");
      return 67;
    }

    //
    // Make the parameter set from the intermediate table.
    //
    fhicl::ParameterSet main_pset;
    try {
      make_ParameterSet(raw_config, main_pset);
    }
    catch (cet::exception const& e) {
      constexpr cet::HorizontalRule rule{36};
      cerr << "ERROR: Failed to create a parameter set from parsed "
              "configuration with exception "
           << e.what() << ".\n";
      cerr << "       Intermediate configuration state follows:\n"
           << rule('-') << '\n'
           << rule('-') << '\n';
      for (auto const& item : raw_config) {
        cerr << item.first << ": " << item.second.to_string() << '\n';
      }
      cerr << rule('-') << '\n' << rule('-') << '\n';
      return 91;
    }
    // Main parameter set must be placed in registry manually.
    try {
      fhicl::ParameterSetRegistry::put(main_pset);
    }
    catch (...) {
      cerr << "Uncaught exception while inserting main parameter set into "
              "registry.\n";
      throw;
    }
    return run_art_common_(main_pset);
  }

  int
  run_art_string_config(string const& config_string)
  {
    //
    // Make the parameter set from the configuration string:
    //
    fhicl::ParameterSet main_pset;
    try {
      // create an intermediate table from the input string
      fhicl::intermediate_table raw_config;
      parse_document(config_string, raw_config);
      // run post-processing
      bpo::variables_map vm;
      BasicPostProcessor bpp;
      bpp.processOptions(vm, raw_config);
      // create the parameter set
      make_ParameterSet(raw_config, main_pset);
    }
    catch (cet::exception& e) {
      constexpr cet::HorizontalRule rule{36};
      cerr << "ERROR: Failed to create a parameter set from an input "
              "configuration string with exception "
           << e.what() << ".\n";
      cerr << "       Input configuration string follows:\n"
           << rule('-') << rule('-') << "\n";
      cerr << config_string << "\n";
      cerr << rule('-') << rule('-') << '\n';
      return 91;
    }
    // Main parameter set must be placed in registry manually.
    try {
      fhicl::ParameterSetRegistry::put(main_pset);
    }
    catch (...) {
      cerr << "Uncaught exception while inserting main parameter set into "
              "registry.\n";
      throw;
    }
    return run_art_common_(main_pset);
  }

  int
  run_art_common_(fhicl::ParameterSet const& main_pset)
  {
#ifdef __linux__
    // Tell the system memory allocator to only use one arena: they
    // are 64 MiB in size, and the default is 8 * num_of_cores.  Using
    // the default means that when using 40 threads we get 40 arenas,
    // which means we have 40 * 64 MiB = 2560 MiB of virtual address
    // space devoted to per-thread heaps!!!
    mallopt(M_ARENA_MAX, 1);
#endif // __linux__
    auto const& services_pset =
      main_pset.get<fhicl::ParameterSet>("services", {});
    auto const& scheduler_pset =
      services_pset.get<fhicl::ParameterSet>("scheduler", {});
    // Handle early configuration-debugging
    auto const debug_processing_mode =
      maybe_output_config(main_pset, scheduler_pset);
    if (debug_processing_mode == debug_processing::debug_config) {
      return detail::info_success();
    }
    //
    // Start the messagefacility
    //
    mf::SetIteration("JobSetup");
    try {
      mf::StartMessageFacility(
        services_pset.get<fhicl::ParameterSet>("message", {}));
    }
    catch (cet::exception const& e) {
      cerr << e.what() << '\n';
      return 69;
    }
    catch (exception const& e) {
      cerr << e.what() << '\n';
      return 70;
    }
    catch (...) {
      cerr << "Caught unknown exception while initializing the message "
              "facility.\n";
      return 71;
    }
    mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";
    //
    // Start the EventProcessor
    //
    int rc{0};
    try {
      EventProcessor ep{main_pset};
      // Behavior of validate_config is to validate FHiCL syntax *and*
      // user-specified configurations of paths, modules, services,
      // etc.  It is thus possible that an exception thrown during
      // construction of the EventProcessor object can have nothing to
      // do with a configuration error.
      if (debug_processing_mode == debug_processing::validate_config) {
        return detail::info_success();
      }
      if (scheduler_pset.has_key("dataDependencyGraph")) {
        return detail::info_success();
      }
      auto ep_rc = ep.runToCompletion();
      if (ep_rc == EventProcessor::epSignal) {
        cerr << "Art has handled signal " << art::shutdown_flag << ".\n";
        if (scheduler_pset.get<bool>("errorOnSIGINT")) {
          rc = 128 + art::shutdown_flag;
        }
      }
    }
    catch (Exception const& e) {
      rc = e.returnCode();
      printArtException(e, "art");
    }
    catch (cet::exception const& e) {
      rc = 65;
      printArtException(e, "art");
    }
    catch (bad_alloc const& bda) {
      rc = 68;
      printBadAllocException("art");
    }
    catch (exception const& e) {
      rc = 66;
      printStdException(e, "art");
    }
    catch (...) {
      rc = 67;
      printUnknownException("art");
    }
    return rc;
  }

} // namespace art
