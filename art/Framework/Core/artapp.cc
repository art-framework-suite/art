// ======================================================================
//
// This is a generic main that can be used with any plugin and a PSet
// script.   See notes in EventProcessor.cpp for details about it.
//
// ======================================================================

#include "TError.h"
#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/IntermediateTablePostProcessor.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Persistency/Common/InitRootHandlers.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "boost/program_options.hpp"
#include "boost/shared_ptr.hpp"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/parse.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <cstring>
#include <exception>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

extern "C" { int artapp(int argc, char* argv[]); }

using fhicl::ParameterSet;

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
      explicit EventProcessorWithSentry() : ep_(0), callEndJob_(false) { }
      explicit EventProcessorWithSentry(std::auto_ptr<art::EventProcessor> ep) :
         ep_(ep),
         callEndJob_(false) { }
      ~EventProcessorWithSentry() {
         if (callEndJob_ && ep_.get()) {
            try {
               ep_->endJob();
            }
            catch (cet::exception& e) {
               //art::printCmsException(e, kProgramName);
            }
            catch (std::bad_alloc& e) {
               //art::printBadAllocException(kProgramName);
            }
            catch (std::exception& e) {
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

      art::EventProcessor* operator->() {
         return ep_.get();
      }
   private:
      std::auto_ptr<art::EventProcessor> ep_;
      bool callEndJob_;
   }; // EventProcessorWithSentry

} // namespace


// -----------------------------------------------

namespace  bpo=boost::program_options;
using std::string;
using std::ostringstream;
using std::ifstream;

int artapp(int argc, char* argv[])
{
  int rc = -1; // we should never return this value!

  // ------------------
  // use the boost command line option processing library to help out
  // with command line options

  ostringstream descstr;

  descstr << argv[0]
          << " <options>";

  bpo::options_description desc(descstr.str());

  desc.add_options()
    ("help,h", "produce help message")
    ("config,c", bpo::value<string>(), "configuration file");

  bpo::options_description all_options("All Options");
  all_options.add(desc);

  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc,argv).options(all_options).run(),vm);
    bpo::notify(vm);
  }
  catch(bpo::error const& e) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": " << e.what() << "\n";
    return 7000;
  }

  if (vm.count("help")) {
    std::cout << desc <<std::endl;
    return 0;
  }

  if (!vm.count("config")) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": no configuration file given.\n"
              << "For usage and an options list, please do '"
              << argv[0] <<  " --help"
              << "'.\n";
    return 7001;
  }

  //
  // Get the parameter set by parsing the configuration file.
  //
  ParameterSet main_pset;
  fhicl::intermediate_table raw_config;
  string config_filename = vm["config"].as<string>();
  ifstream config_stream(config_filename.c_str());
  if (!fhicl::parse_document(config_stream, raw_config)) {
     std::cerr << "Failed to parse the configuration file '"
               << config_filename
               << "'\n";
     return 7002;
  } else if ( raw_config.empty() ) {
     std::cerr << "INFO: provided configuration file '"
               << config_filename.c_str()
               << "' is empty: using minimal defaults.\n";
  }

  //
  // Make the parameter set from the intermediate table with any
  // appropriate post-processing:
  //
  art::IntermediateTablePostProcessor itpp;

  if (!itpp(raw_config, main_pset)) {
     std::cerr << "Failed to create a parameter set from parsed "
               << "intermediate representation of configuration file '"
                << config_filename
                << "'\n";
      return 7003;
  }

  ParameterSet services_pset = main_pset.get<ParameterSet>("services", ParameterSet());
  ParameterSet scheduler_pset = services_pset.get<ParameterSet>("scheduler", ParameterSet());

  //
  // Start the messagefacility
  //

  mf::MessageDrop::instance()->jobMode = std::string("analysis");

  mf::StartMessageFacility(mf::MessageFacilityService::MultiThread,
                           services_pset.get<ParameterSet>("message", ParameterSet()));

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
  try {
    std::auto_ptr<art::EventProcessor> 
       procP(new
             art::EventProcessor(main_pset));
    EventProcessorWithSentry procTmp(procP);
    proc = procTmp;
    proc->beginJob();
    proc.on();
    bool onlineStateTransitions = false;
    proc->runToCompletion(onlineStateTransitions);
    proc.off();
    proc->endJob();
    rc = 0;
  }
  catch (art::Exception& e) {
    rc = e.returnCode();
    art::printCmsException(e, "art"); // , "Thing1", rc);
  }
  catch (cet::exception& e) {
    rc = 8001;
    art::printCmsException(e, "art"); // , "Thing2", rc);
  }
  catch(std::bad_alloc& bda) {
    rc = 8004;
    art::printBadAllocException("art"); // , "Thing3", rc);
  }
  catch (std::exception& e) {
    rc = 8002;
    art::printStdException(e, "art"); // , "Thing4", rc);
  }
  catch (...) {
    rc = 8003;
    art::printUnknownException("art"); // , "Thing5", rc);
  }

  return rc;
}  // artapp
