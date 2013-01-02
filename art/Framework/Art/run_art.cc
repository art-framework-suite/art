#include "art/Framework/Art/run_art.h"

#include "art/Framework/Art/BasicOptionsHandler.h"
#include "art/Framework/Art/BasicPostProcessor.h"
#include "art/Framework/Art/InitRootHandlers.h"
#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "boost/program_options.hpp"
#include "boost/regex.hpp"
#include "TError.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace bpo = boost::program_options;

// -----------------------------------------------
namespace {
  struct RootErrorHandlerSentry {
    RootErrorHandlerSentry(bool reset) {
      art::setRootErrorHandler(reset);
    }
    ~RootErrorHandlerSentry() {
      SetErrorHandler(DefaultErrorHandler);
    }
  };

  class EventProcessorWithSentry {
  public:
    explicit EventProcessorWithSentry() : ep_(), callEndJob_(false) { }
    explicit EventProcessorWithSentry(std::unique_ptr<art::EventProcessor> && ep) :
      ep_(std::move(ep)),
      callEndJob_(false) { }
    EventProcessorWithSentry(EventProcessorWithSentry &&) = default;
    EventProcessorWithSentry &
    operator =(EventProcessorWithSentry &&) = default;

    ~EventProcessorWithSentry() {
      if (callEndJob_ && ep_.get()) {
        try {
          ep_->endJob();
        }
        catch (cet::exception & e) {
          //art::printArtException(e, kProgramName);
        }
        catch (std::bad_alloc & e) {
          //art::printBadAllocException(kProgramName);
        }
        catch (std::exception & e) {
          //art::printStdException(e, kProgramName);
        }
        catch (...) {
          //art::printUnknownException(kProgramName);
        }
      }
    }
    void on() {
      callEndJob_ = true;
    }
    void off() {
      callEndJob_ = false;
    }

    art::EventProcessor * operator->() {
      return ep_.get();
    }
  private:
    std::unique_ptr<art::EventProcessor> ep_;
    bool callEndJob_;
  }; // EventProcessorWithSentry

} // namespace

int art::run_art(int argc,
                 char ** argv,
                 bpo::options_description & in_desc,
                 cet::filepath_maker & lookupPolicy,
                 art::OptionsHandlers && handlers)
{
  std::ostringstream descstr;
  descstr << "Usage: "
          << argv[0]
          << " <-c <config-file>> <other-options> [<source-file>]+\n\n"
          << "Allowed options";
  bpo::options_description all_desc(descstr.str());
  all_desc.add(in_desc);
  // BasicOptionsHandler should always be first in the list!
  handlers.emplace(handlers.begin(),
                   new BasicOptionsHandler(all_desc, lookupPolicy));
  handlers.emplace_back(new BasicPostProcessor);
  // This must be added separately: how to deal with any non-option arguments.
  bpo::positional_options_description pd;
  // A single non-option argument will be taken to be the source data file.
  pd.add("source", -1);
  // Parse the command line.
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(all_desc).positional(pd).run(),
               vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": " << e.what() << "\n";
    return 7000;
  }
  // Preliminary argument checking.
for (auto & handler : handlers) {
    auto result = handler->checkOptions(vm);
    if (result != 0) {
      return result;
    }
  }
  // Processing of arguments and post-processing of config.
  fhicl::intermediate_table raw_config;
for (auto & handler : handlers) {
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
  catch (cet::exception & e) {
    std::cerr << "ERROR: Failed to create a parameter set from parsed configuration with exception "
              << e.what()
              << ".\n";
    std::cerr << "       Intermediate configuration state follows:\n"
              << "------------------------------------"
              << "------------------------------------"
              << "\n";
  for (auto const & item : raw_config) {
      std::cerr << item.first << ": " << item.second.to_string() << "\n";
    }
    std::cerr
        << "------------------------------------"
        << "------------------------------------"
        << "\n";
    return 7003;
  }

  return run_art_common_(main_pset);
}

int art::run_art_string_config(const std::string& config_string)
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
  catch (cet::exception & e) {
    std::cerr << "ERROR: Failed to create a parameter set from an input configuration string with exception "
              << e.what()
              << ".\n";
    std::cerr << "       Input configuration string follows:\n"
              << "------------------------------------"
              << "------------------------------------"
              << "\n";
    std::cerr << config_string << "\n";
    std::cerr
        << "------------------------------------"
        << "------------------------------------"
        << "\n";
    return 7003;
  }

  return run_art_common_(main_pset);
}

int art::run_art_common_(fhicl::ParameterSet main_pset)
{
  char const * debug_config (getenv("ART_DEBUG_CONFIG"));
  if (debug_config != nullptr) {
    bool isFilename(false);
    try {
      // GCC 4.7.1 cannot handle complex character classes -- use boost::regex instead.
      isFilename = boost::regex_match(debug_config, boost::regex("[[:alpha:]/\\.].*"));
    }
    catch(boost::regex_error e) {
      std::cerr << "REGEX ERROR: " << e.code() << ".\n";
    }
    if (isFilename) {
      std::cerr << "** ART_DEBUG_CONFIG is defined: config debug output to file "
                << debug_config
                << " **\n";
      std::ofstream dc(debug_config);
      if (dc) {
        dc << main_pset.to_indented_string() << "\n";
        return 1;
      } else {
        std::cerr << "Output of config to " << debug_config << " failed: fallback to stderr.\n";
      }
    } else {
      std::cerr << "** ART_DEBUG_CONFIG is defined: config debug output follows **\n";
    }
    std::cerr << main_pset.to_indented_string() << "\n";
    return 1;
  }
  fhicl::ParameterSet
  services_pset(main_pset.get<fhicl::ParameterSet>("services",
                fhicl::ParameterSet()));
  fhicl::ParameterSet
  scheduler_pset(services_pset.get<fhicl::ParameterSet>("scheduler",
                 fhicl::ParameterSet()));
  //
  // Start the messagefacility
  //
  mf::MessageDrop::instance()->jobMode = std::string("analysis");
  mf::MessageDrop::instance()->runEvent = std::string("JobSetup");
  mf::StartMessageFacility(mf::MessageFacilityService::MultiThread,
                           services_pset.get<fhicl::ParameterSet>("message",
                               fhicl::ParameterSet()));
  mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";
  //
  // Initialize:
  //   unix signal facility
  art::setupSignals(scheduler_pset.get<bool>("enableSigInt", true));
  //   init root handlers facility
  if (scheduler_pset.get<bool>("unloadRootSigHandler", true)) {
    art::unloadRootSigHandler();
  }
  RootErrorHandlerSentry re_sentry(scheduler_pset.get<bool>("resetRootErrHandler", true));
  // Load all dictionaries.
  art::RootDictionaryManager rdm;
  art::completeRootHandlers();
  art::ServiceToken dummyToken;
  // TODO: Possibly remove addServices -- we have already made
  // most of them. Have to see how the module factory interacts
  // with the current module facility.
  // processDesc->addServices(defaultServices, forcedServices);
  //
  // Now create the EventProcessor
  //
  EventProcessorWithSentry proc;
  int rc = -1;
  try {
    std::unique_ptr<art::EventProcessor>
    procP(new
          art::EventProcessor(main_pset));
    EventProcessorWithSentry procTmp(std::move(procP));
    proc = std::move(procTmp);
    proc->beginJob();
    proc.on();
    proc->runToCompletion();
    proc.off();
    proc->endJob();
    rc = 0;
  }
  catch (art::Exception & e) {
    rc = e.returnCode();
    art::printArtException(e, "art"); // , "Thing1", rc);
  }
  catch (cet::exception & e) {
    rc = 8001;
    art::printArtException(e, "art"); // , "Thing2", rc);
  }
  catch (std::bad_alloc & bda) {
    rc = 8004;
    art::printBadAllocException("art"); // , "Thing3", rc);
  }
  catch (std::exception & e) {
    rc = 8002;
    art::printStdException(e, "art"); // , "Thing4", rc);
  }
  catch (...) {
    rc = 8003;
    art::printUnknownException("art"); // , "Thing5", rc);
  }
  return rc;
}
