#include "art/Framework/Art/run_art.h"

#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/Art/detail/info_success.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Framework/IO/Root/InitRootHandlers.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "cetlib/ostream_handle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/detail/print_mode.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TError.h"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace bpo = boost::program_options;

// -----------------------------------------------
namespace {
  struct RootErrorHandlerSentry {
    RootErrorHandlerSentry(bool const reset)
    {
      art::setRootErrorHandler(reset);
    }
    ~RootErrorHandlerSentry() { SetErrorHandler(DefaultErrorHandler); }
  };

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
    std::underlying_type_t<debug_processing> i{};
    for (auto const debugProcessing :
         {"configOut", "debugConfig", "validateConfig"}) {
      auto const j = i++;
      if (!scheduler_pset.has_key(debugProcessing))
        continue;

      // Handle the backwards compatibility case, where "configOut"
      // was associated with a filename in older configurations.
      if (scheduler_pset.is_key_to_atom(debugProcessing)) {
        assert(std::strcmp(debugProcessing, "configOut") == 0);
        auto const filename = scheduler_pset.get<std::string>("configOut");
        std::cerr << banner(filename);
        auto os = make_ostream_handle(filename);
        os << main_pset.to_indented_string(0, fhicl::detail::print_mode::raw);
        return debug_processing::config_out;
      }

      auto const& debug_table =
        scheduler_pset.get<fhicl::ParameterSet>(debugProcessing);
      auto const filename = debug_table.get<std::string>("fileName");
      auto const mode = debug_table.get<std::string>("printMode");
      std::cerr << banner(filename);
      auto os = make_ostream_handle(filename);
      os << main_pset.to_indented_string(0, get_print_mode(mode));
      return static_cast<debug_processing>(j);
    }
    return debug_processing::none;
  }

} // namespace

int
art::run_art(int argc,
             char** argv,
             bpo::options_description& in_desc,
             cet::filepath_maker& lookupPolicy,
             art::OptionsHandlers&& handlers)
{
  std::ostringstream descstr;
  descstr << '\n'
          << "Usage"
          << ": " << boost::filesystem::path(argv[0]).filename().native()
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
    std::cerr << "Exception from command line processing in " << argv[0] << ": "
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

  // Make the parameter set from the intermediate table.
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
  return run_art_common_(main_pset);
}

int
art::run_art_string_config(std::string const& config_string)
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
  return run_art_common_(main_pset);
}

int
art::run_art_common_(fhicl::ParameterSet const& main_pset)
{
  auto const& services_pset =
    main_pset.get<fhicl::ParameterSet>("services", {});
  auto const& scheduler_pset =
    services_pset.get<fhicl::ParameterSet>("scheduler", {});

  // Handle early configuration-debugging
  auto const debug_processing_mode =
    maybe_output_config(main_pset, scheduler_pset);
  if (debug_processing_mode == debug_processing::debug_config) {
    return detail::info_success(); // Bail out early
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
    std::cerr
      << "Caught unknown exception while initializing the message facility.\n";
    return 71;
  }

  mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";
  //
  // Initialize:
  //   unix signal facility
  art::setupSignals(scheduler_pset.get<bool>("enableSigInt", true));
  //   init root handlers facility
  if (scheduler_pset.get<bool>("unloadRootSigHandler", true)) {
    art::unloadRootSigHandler();
  }
  RootErrorHandlerSentry re_sentry{
    scheduler_pset.get<bool>("resetRootErrHandler", true)};
  // Load all dictionaries.
  if (scheduler_pset.get<bool>("debugDictionaries", false)) {
    throw Exception(errors::UnimplementedFeature)
      << "debugDictionaries not yet implemented for ROOT 6.\n";
  }
  art::completeRootHandlers();

  int rc{0};
  try {
    EventProcessor ep{main_pset};
    // Behavior of validate_config is to validate FHiCL syntax *and*
    // user-specified configurations of paths, modules, services, etc.
    // It is thus possible that an exception thrown during
    // construction of the EventProcessor object can have nothing to
    // do with a configuration error.
    if (debug_processing_mode == debug_processing::validate_config) {
      return detail::info_success(); // Bail out early
    }
    if (ep.runToCompletion() == EventProcessor::epSignal) {
      std::cerr << "Art has handled signal " << art::shutdown_flag << ".\n";
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
