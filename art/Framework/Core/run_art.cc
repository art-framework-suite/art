#include "art/Framework/Core/run_art.h"

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
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace fhicl;

using std::string;
using std::ostringstream;
using std::ifstream;
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
               //art::printArtException(e, kProgramName);
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


int art::run_art(intermediate_table raw_config) {
   //
   // Make the parameter set from the intermediate table with any
   // appropriate post-processing:
   //
   ParameterSet main_pset;
   art::IntermediateTablePostProcessor itpp;
   itpp.apply(raw_config);
   if (!make_ParameterSet(raw_config, main_pset)) {
      std::cerr << "ERROR: Failed to create a parameter set from parsed configuration.\n";
      std::cerr << "       Intermediate configuration state follows:\n"
                << "------------------------------------"
                << "------------------------------------"
                << "\n";         
      for (extended_value::table_t::const_iterator
              i = raw_config.begin(),
              end_iter = raw_config.end();
           i != end_iter;
           ++i) {
         std::cerr << i->first << ": " << i->second.to_string() << "\n";
      }
      std::cerr
         << "------------------------------------"
         << "------------------------------------"
         << "\n";         
      return 7003;
   }

   char const *debug_config = getenv("ART_DEBUG_CONFIG");
   if (debug_config != nullptr) {
      std::cerr << "** ART_DEBUG_CONFIG is defined: config debug output follows **\n";
      std::cerr << main_pset.to_string() << "\n";
      return 1;
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
   int rc = -1;
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
      art::printArtException(e, "art"); // , "Thing1", rc);
   }
   catch (cet::exception& e) {
      rc = 8001;
      art::printArtException(e, "art"); // , "Thing2", rc);
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
