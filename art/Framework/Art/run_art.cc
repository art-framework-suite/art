#include "art/Framework/Art/run_art.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Framework/Art/detail/info_success.h"
#include "art/Framework/Art/detail/output_to.h"
#include "art/Framework/Art/detail/print_config_summary.h"
#include "art/Framework/Art/detail/prune_configuration.h"
#include "art/Framework/Core/detail/EnabledModules.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/UnixSignalHandlers.h"
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
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <string>

#ifdef __linux__
#include <malloc.h>
#endif // __linux__

using namespace std;
using namespace string_literals;

using art::detail::fhicl_key;

namespace {

  cet::ostream_handle
  make_ostream_handle(std::string const& filename)
  {
    assert(!filename.empty());
    if (art::detail::output_to_stderr(filename)) {
      return cet::ostream_handle{std::cerr};
    } else if (art::detail::output_to_stdout(filename)) {
      return cet::ostream_handle{std::cout};
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
    bool const to_terminal = art::detail::output_to_stderr(filename) ||
                             art::detail::output_to_stdout(filename);
    result +=
      to_terminal ? "follows" : std::string("to file '" + filename + "'");
    result += " **\n";
    return result;
  }

  void
  print_config(fhicl::ParameterSet const& main_pset,
               std::string const& output_file,
               std::string const& mode)
  {
    std::cerr << banner(output_file);
    auto os = make_ostream_handle(output_file);
    os << main_pset.to_indented_string(0, get_print_mode(mode));
  }

  enum class debug_processing : std::size_t {
    config_summary,
    config_out,
    debug_config,
    validate_config,
    none
  };

  debug_processing
  debug_processing_mode(fhicl::ParameterSet const& scheduler_pset)
  {
    if (not scheduler_pset.has_key("debug")) {
      return debug_processing::none;
    }

    auto const processing_options = {
      "config-summary", "config-out", "debug-config", "validate-config"};

    auto const option = scheduler_pset.get<std::string>("debug.option");
    auto pos = cet::find_in_all(processing_options, option);
    if (pos == cend(processing_options)) {
      throw art::Exception{
        art::errors::Configuration,
        "An error was encountered while processing debugging options"}
        << "The debugging option '" << option << "' is not supported.\n"
        << "If you did not explicitly provide this value in your configuration "
           "file, please contact artists@fnal.gov and report this error.  "
           "Otherwise,\n"
        << "choose from 'configSummary', 'configOut', 'debugConfig', or "
           "'validateConfig'.\n";
    }

    auto const index = std::distance(cbegin(processing_options), pos);
    return static_cast<debug_processing>(index);
  }

} // unnamed namespace

namespace bpo = boost::program_options;

namespace art {

  int
  run_art(int argc,
          char** argv,
          bpo::options_description& all_desc,
          OptionsHandlers&& handlers)
  {
    // This must be added separately: how to deal with any non-option arguments.
    bpo::positional_options_description pd;
    // A single non-option argument will be taken to be the source data file.
    pd.add("source", -1);
    // Parse the command line.
    bpo::variables_map vm;
    try {
      bpo::store(bpo::command_line_parser(argc, argv)
                   .options(all_desc)
                   .style(bpo::command_line_style::default_style &
                          ~bpo::command_line_style::allow_guessing)
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
      auto const result = handler->checkOptions(vm);
      if (result != 0) {
        return result;
      }
    }
    // Processing of arguments and post-processing of config.
    fhicl::intermediate_table raw_config;
    for (auto& handler : handlers) {
      auto const result = handler->processOptions(vm, raw_config);
      if (result != 0) {
        return result;
      }
    }

    // If configuration pruning has been enabled, remove unused module
    // configurations.
    using detail::exists_outside_prolog;
    auto const scheduler_key = fhicl_key("services", "scheduler");
    auto enabled_modules = detail::EnabledModules::none();
    assert(exists_outside_prolog(raw_config, scheduler_key));
    try {
      auto const pruneConfigKey = fhicl_key(scheduler_key, "pruneConfig");
      auto const reportUnusedKey = fhicl_key(scheduler_key, "reportUnused");
      assert(exists_outside_prolog(raw_config, pruneConfigKey));
      assert(exists_outside_prolog(raw_config, reportUnusedKey));
      bool const prune_config = raw_config.get<bool>(pruneConfigKey);
      bool const report_unused = raw_config.get<bool>(reportUnusedKey);
      enabled_modules = detail::prune_config_if_enabled(
        prune_config, report_unused, raw_config);
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
      main_pset = fhicl::ParameterSet::make(raw_config);
    }
    catch (cet::exception const& e) {
      constexpr cet::HorizontalRule rule{36};
      cerr << "ERROR: Failed to create a parameter set from parsed "
              "configuration with exception "
           << e.what() << ".\n";
      cerr << "       Intermediate configuration state follows:\n"
           << rule('-') << '\n'
           << rule('-') << '\n';
      for (auto const& [key, value] : raw_config) {
        cerr << key << ": " << value.to_string() << '\n';
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

#ifdef __linux__
    // Tell the system memory allocator to only use one arena: they
    // are 64 MiB in size, and the default is 8 * num_of_cores.  Using
    // the default means that when using 40 threads we get 40 arenas,
    // which means we have 40 * 64 MiB = 2560 MiB of virtual address
    // space devoted to per-thread heaps!!!
    mallopt(M_ARENA_MAX, 1);
#endif // __linux__

    auto const services_pset =
      main_pset.get<fhicl::ParameterSet>("services", {});
    auto const scheduler_pset =
      services_pset.get<fhicl::ParameterSet>("scheduler", {});

    // Handle early configuration-debugging
    auto const debug_mode = debug_processing_mode(scheduler_pset);
    if (debug_mode != debug_processing::none) {
      auto const debug_pset = scheduler_pset.get<fhicl::ParameterSet>("debug");
      auto const filename = debug_pset.get<std::string>("fileName");
      auto const mode = debug_pset.get<std::string>("printMode");

      switch (debug_mode) {
      case debug_processing::validate_config: {
        [[fallthrough]];
      }
      case debug_processing::config_out: {
        print_config(main_pset, filename, mode);
        break;
      }
      case debug_processing::debug_config: {
        print_config(main_pset, filename, mode);
        return detail::info_success();
      }
      case debug_processing::config_summary: {
        detail::print_config_summary(main_pset, mode, enabled_modules);
        return detail::info_success();
      }
      case debug_processing::none:
        break;
      }
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
      EventProcessor ep{std::move(main_pset), std::move(enabled_modules)};
      // Behavior of validate_config is to validate FHiCL syntax *and*
      // user-specified configurations of paths, modules, services,
      // etc.  It is thus possible that an exception thrown during
      // construction of the EventProcessor object can have nothing to
      // do with a configuration error.
      if (debug_mode == debug_processing::validate_config) {
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
    catch (detail::collected_exception const& e) {
      rc = 1;
      // LogSystem already adds a newline, so trim the one that's
      // already in the exception message.
      std::string const msg{e.what()};
      mf::LogSystem("ArtException") << msg.substr(0, msg.find_last_of("\n"));
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
