#include "art/Framework/Art/run_art.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Framework/IO/Root/InitRootHandlers.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TError.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <malloc.h>

namespace bpo = boost::program_options;

namespace {

  struct RootErrorHandlerSentry {
    RootErrorHandlerSentry(bool const reset)
    {
      art::setRootErrorHandler(reset);
    }
    ~RootErrorHandlerSentry() { SetErrorHandler(DefaultErrorHandler); }
  };

} // unnamed namespace

namespace art {

  int
  run_art(int argc,
          char** argv,
          bpo::options_description& in_desc,
          cet::filepath_maker& lookupPolicy,
          OptionsHandlers&& handlers,
          detail::DebugOutput&& dbg)
  {
    std::ostringstream descstr;
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
      std::cerr << "Exception from command line processing in " << argv[0]
                << ": " << e.what() << "\n";
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
    //
    // Make the parameter set from the intermediate table:
    //
    fhicl::ParameterSet main_pset;
    try {
      make_ParameterSet(raw_config, main_pset);
    }
    catch (cet::exception const& e) {
      constexpr cet::HorizontalRule rule{36};
      std::cerr << "ERROR: Failed to create a parameter set from parsed "
                   "configuration with exception "
                << e.what() << ".\n";
      std::cerr << "       Intermediate configuration state follows:\n"
                << rule('-') << '\n'
                << rule('-') << '\n';
      for (auto const& item : raw_config) {
        std::cerr << item.first << ": " << item.second.to_string() << '\n';
      }
      std::cerr << rule('-') << '\n' << rule('-') << '\n';
      return 91;
    }

    // Main parameter set must be placed in registry manually.
    try {
      fhicl::ParameterSetRegistry::put(main_pset);
    }
    catch (...) {
      std::cerr << "Uncaught exception while inserting main parameter set into "
                   "registry.\n";
      throw;
    }
    return run_art_common_(main_pset, std::move(dbg));
  }

  int
  run_art_string_config(std::string const& config_string)
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
      std::cerr << "ERROR: Failed to create a parameter set from an input "
                   "configuration string with exception "
                << e.what() << ".\n";
      std::cerr << "       Input configuration string follows:\n"
                << rule('-') << rule('-') << "\n";
      std::cerr << config_string << "\n";
      std::cerr << rule('-') << rule('-') << '\n';
      return 91;
    }
    // Main parameter set must be placed in registry manually.
    try {
      fhicl::ParameterSetRegistry::put(main_pset);
    }
    catch (...) {
      std::cerr << "Uncaught exception while inserting main parameter set into "
                   "registry.\n";
      throw;
    }
    return run_art_common_(main_pset, detail::DebugOutput{});
  }

  int
  run_art_common_(fhicl::ParameterSet const& main_pset,
                  detail::DebugOutput debug)
  {
#ifdef __linux__
    // FIXME: Figure out is we should do something similar for Darwin

    // Tell the system memory allocator to only use one arena: they
    // are 64 MiB in size, and the default is 8 * num_of_cores.  Using
    // the default means that when using 40 threads we get 40 arenas,
    // which means we have 40 * 64 MiB = 2560 MiB of virtual address
    // space devoted to per-thread heaps!!!
    mallopt(M_ARENA_MAX, 1);
#endif
    auto const& services_pset =
      main_pset.get<fhicl::ParameterSet>("services", {});
    auto const& scheduler_pset =
      services_pset.get<fhicl::ParameterSet>("scheduler", {});
    if (debug && debug.preempting()) {
      std::cerr << debug.banner();
      debug.stream() << main_pset.to_indented_string(0, debug.mode());
      return 1;
    }
    //
    // Start the messagefacility
    //
    mf::MessageDrop::jobMode = std::string("analysis");
    mf::MessageDrop::instance()->iteration = std::string("JobSetup");
    try {
      mf::StartMessageFacility(
        services_pset.get<fhicl::ParameterSet>("message", {}));
    }
    catch (cet::exception const& e) {
      std::cerr << e.what() << '\n';
      return 69;
    }
    catch (std::exception const& e) {
      std::cerr << e.what() << '\n';
      return 70;
    }
    catch (...) {
      std::cerr << "Caught unknown exception while initializing the message "
                   "facility.\n";
      return 71;
    }
    mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";
    //
    // Configuration output (non-preempting)
    //
    if (debug && !debug.preempting()) {
      if (debug.stream_is_valid()) {
        debug.stream() << main_pset.to_indented_string(0, debug.mode());
        mf::LogInfo("ConfigOut") << "Post-processed configuration written to "
                                 << debug.filename() << ".\n";
      } else { // Error!
        throw Exception(errors::Configuration)
          << "Unable to write post-processed configuration to specified file "
          << debug.filename() << ".\n";
      }
    }
    //
    // Initialize:
    //   unix signal facility
    setupSignals(scheduler_pset.get<bool>("enableSigInt", true));
    //   init root handlers facility
    if (scheduler_pset.get<bool>("unloadRootSigHandler", true)) {
      unloadRootSigHandler();
    }
    RootErrorHandlerSentry re_sentry{
      scheduler_pset.get<bool>("resetRootErrHandler", true)};
    // Load all dictionaries.
    if (scheduler_pset.get<bool>("debugDictionaries", false)) {
      throw Exception(errors::UnimplementedFeature)
        << "debugDictionaries not yet implemented for ROOT 6.\n";
    }
    completeRootHandlers();
    int rc = 0;
    try {
      EventProcessor ep{main_pset};
      if (ep.runToCompletion() == EventProcessor::epSignal) {
        std::cerr << "Art has handled signal " << shutdown_flag << ".\n";
        if (scheduler_pset.get<bool>("errorOnSIGINT")) {
          rc = 128 + shutdown_flag;
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
    catch (std::bad_alloc const& bda) {
      rc = 68;
      printBadAllocException("art");
    }
    catch (std::exception const& e) {
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
