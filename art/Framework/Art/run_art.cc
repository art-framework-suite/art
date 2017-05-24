#include "art/Framework/Art/run_art.h"

#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/Art/InitRootHandlers.h"
#include "art/Framework/Art/detail/handle_deprecated_configs.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/HorizontalRule.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "TError.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace bpo = boost::program_options;

// -----------------------------------------------
namespace {
  struct RootErrorHandlerSentry {
    RootErrorHandlerSentry(bool const reset) {
      art::setRootErrorHandler(reset);
    }
    ~RootErrorHandlerSentry() {
      SetErrorHandler(DefaultErrorHandler);
    }
  };
} // namespace

int art::run_art(int argc,
                 char** argv,
                 bpo::options_description& in_desc,
                 cet::filepath_maker& lookupPolicy,
                 art::OptionsHandlers&& handlers,
                 art::detail::DebugOutput&& dbg)
{
  std::ostringstream descstr;
  descstr << '\n' << "Usage" << ": "
          << boost::filesystem::path(argv[0]).filename().native()
          << " <-c <config-file>> <other-options> [<source-file>]+\n\n"
          << "Basic options";
  bpo::options_description all_desc {descstr.str()};
  all_desc.add(in_desc);
  // BasicOptionsHandler should always be first in the list!
  handlers.emplace(handlers.begin(), new BasicOptionsHandler(all_desc, lookupPolicy));
  // BasicPostProcessor should be last.
  handlers.emplace_back(new BasicPostProcessor);
  // This must be added separately: how to deal with any non-option arguments.
  bpo::positional_options_description pd;
  // A single non-option argument will be taken to be the source data file.
  pd.add("source", -1);
  // Parse the command line.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(all_desc).positional(pd).run(), vm);
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
  // Handle deprecated configurations
  try {
    detail::handle_deprecated_configs(raw_config);
  }
  catch(art::Exception const& e) {
    std::cerr << e.what();
    return 89;
  }

  //
  // Make the parameter set from the intermediate table:
  //
  fhicl::ParameterSet main_pset;
  try {
    make_ParameterSet(raw_config, main_pset);
  }
  catch (cet::exception& e) {
    HorizontalRule const rule{36};
    std::cerr << "ERROR: Failed to create a parameter set from parsed configuration with exception "
              << e.what()
              << ".\n";
    std::cerr << "       Intermediate configuration state follows:\n"
              << rule('-')
              << rule('-')
              << '\n';
    for (auto const& item : raw_config) {
      std::cerr << item.first << ": " << item.second.to_string() << '\n';
    }
    std::cerr << rule('-')
              << rule('-')
              << '\n';
    return 91;
  }
  // Main parameter set must be placed in registry manually.
  try {
    fhicl::ParameterSetRegistry::put(main_pset);
  }
  catch (...) {
    std::cerr << "Uncaught exception while inserting main parameter set into registry.\n";
    throw;
  }
  return run_art_common_(main_pset, std::move(dbg));
}

int art::run_art_string_config(std::string const& config_string)
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
    HorizontalRule const rule{36};
    std::cerr << "ERROR: Failed to create a parameter set from an input configuration string with exception "
              << e.what()
              << ".\n";
    std::cerr << "       Input configuration string follows:\n"
              << rule('-')
              << rule('-')
              << "\n";
    std::cerr << config_string << "\n";
    std::cerr << rule('-')
              << rule('-')
              << '\n';
    return 91;
  }
  // Main parameter set must be placed in registry manually.
  try {
    fhicl::ParameterSetRegistry::put(main_pset);
  }
  catch (...) {
    std::cerr << "Uncaught exception while inserting main parameter set into registry.\n";
    throw;
  }
  return run_art_common_(main_pset, art::detail::DebugOutput{});
}

int art::run_art_common_(fhicl::ParameterSet const& main_pset, art::detail::DebugOutput debug)
{
  auto const& services_pset = main_pset.get<fhicl::ParameterSet>("services",{});
  auto const& scheduler_pset = services_pset.get<fhicl::ParameterSet>("scheduler",{});

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
  mf::StartMessageFacility(services_pset.get<fhicl::ParameterSet>("message",{}));
  mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";
  //
  // Configuration output (non-preempting)
  //
  if (debug && !debug.preempting()) {
    if (debug.stream_is_valid()) {
      debug.stream() << main_pset.to_indented_string(0, debug.mode());
      mf::LogInfo("ConfigOut") << "Post-processed configuration written to "
                               << debug.filename()
                               << ".\n";
    }
    else { // Error!
      throw Exception(errors::Configuration)
        << "Unable to write post-processed configuration to specified file "
        << debug.filename()
        << ".\n";
    }
  }
  //
  // Initialize:
  //   unix signal facility
  art::setupSignals(scheduler_pset.get<bool>("enableSigInt", true));
  //   init root handlers facility
  if (scheduler_pset.get<bool>("unloadRootSigHandler", true)) {
    art::unloadRootSigHandler();
  }
  RootErrorHandlerSentry re_sentry {scheduler_pset.get<bool>("resetRootErrHandler", true)};
  // Load all dictionaries.
  if (scheduler_pset.get<bool>("debugDictionaries", false)) {
    throw Exception(errors::UnimplementedFeature)
      << "debugDictionaries not yet implemented for ROOT 6.\n";
  }
  art::completeRootHandlers();

  int rc {0};
  try {
    EventProcessor ep {main_pset};
    if (ep.runToCompletion() == EventProcessor::epSignal) {
      std::cerr << "Art has handled signal "
                << art::shutdown_flag
                << ".\n";
      if (scheduler_pset.get<bool>("errorOnSIGINT"))
        rc = 128 + art::shutdown_flag;
    }
  }
  catch (art::Exception const& e) {
    rc = e.returnCode();
    art::printArtException(e, "art");
  }
  catch (cet::exception const& e) {
    rc = 65;
    art::printArtException(e, "art");
  }
  catch (std::bad_alloc const& bda) {
    rc = 68;
    art::printBadAllocException("art");
  }
  catch (std::exception const& e) {
    rc = 66;
    art::printStdException(e, "art");
  }
  catch (...) {
    rc = 67;
    art::printUnknownException("art");
  }
  return rc;
}
