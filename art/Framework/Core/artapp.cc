/*----------------------------------------------------------------------

This is a generic main that can be used with any plugin and a
PSet script.   See notes in EventProcessor.cpp for details about
it.

----------------------------------------------------------------------*/


#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/PresenceFactory.h"
#include "art/Framework/PluginManager/standard.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
//#include "art/ParameterSet/MakeParameterSets.h"
#include "cetlib/exception.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/Presence.h"
#include "art/Utilities/RootHandlers.h"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "TError.h"
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


static char const* const kParameterSetCommandOpt = "parameter-set,p";
static char const* const kHelpOpt = "help";
static char const* const kHelpCommandOpt = "help,h";

namespace  bpo=boost::program_options;

// -----------------------------------------------
namespace {
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
  };
}

extern "C" { int art_main(int argc, char* argv[]); }

int art_main(int argc, char* argv[])
{

  //
  // We must initialize the plug-in manager first
  // TODO: Replace initialization of the plugin manager.
  try {
    artplugin::PluginManager::configure(artplugin::standard::config());
  }
  catch(cet::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::string descString(argv[0]);
  descString += " [options] [--";
  descString += "parameter-set";
  descString += "] config_file \nAllowed options";
  bpo::options_description desc(descString);

  desc.add_options()
    (kHelpCommandOpt, "produce help message")
    (kParameterSetCommandOpt, bpo::value<std::string>(), "configuration file");

  bpo::options_description all_options("All Options");
  all_options.add(desc);

  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc,argv).options(all_options).run(),vm);
    bpo::notify(vm);
  }
  catch(bpo::error const& iException) {
    std::cerr << "Exception from command line processing in " << argv[0]
	      << ": " << iException.what();
    return 7000;
  }

  if (vm.count(kHelpOpt)) {
    std::cout << desc <<std::endl;
    return 0;
  }

  if (!vm.count("parameter-set")) {
    std::cerr << "Exception from command line processing in " << argv[0]
	      << ": no configuration file given.\n"
	      << "For usage and an options list, please do '"
	      << argv[0] <<  " --" << kHelpOpt
	      << "'.";
    return 7001;
  }

  //
  // Get the parameter set from parsing the configuration file.
  //
  fhicl::ParameterSet main_pset, ancillary_pset;
  fhicl::Parser::Parse(vm["main-parameter-set"].as<std::string>(), main_pset);
  fhicl::Parser::Parse(vm["ancillary-parameter-set"].as<std::string>(), ancillary_pset);


  //
  // Start the messagefacility
  //
  mf::start_me(multithread, ancillary_pset.get<fhicl::ParameterSet>("message_facility"));

  //
  // Initialize:
  //   unix signal facility
  //   enable fpe facility
  //   init root handlers facility

  // Initialize:
  //   load all dictionaries facility
  //   current module facility

  art::ServiceToken dummyToken;

  // TODO: Possibly remove addServices -- we have already made
  // most of them. Have to see how the module factory interacts
  // with the current module facility.
  // processDesc->addServices(defaultServices, forcedServices);

  //
  // Now create the EventProcessor
  //
  EventProcessorWithSentry proc;
  int rc = -1; // we should never return this value!
  try {
    std::auto_ptr<art::EventProcessor> procP;
#if 0
        procP(new
              art::EventProcessor(processDesc, jobReportToken,
                             art::serviceregistry::kTokenOverrides));
#endif  // 0
    EventProcessorWithSentry procTmp(procP);
    proc = procTmp;
    proc->beginJob();
    proc.on();
    bool onlineStateTransitions = false;
    proc->runToCompletion(onlineStateTransitions);
    proc.off();
    proc->endJob();
    rc = 0;
    // Disable Root Error Handler so we do not throw because of ROOT errors.
    art::ServiceToken token = proc->getToken();
    art::ServiceRegistry::Operate operate(token);
    art::Service<art::RootHandlers> rootHandler;
    rootHandler->disableErrorHandler();
  }
  catch (art::Exception& e) {
    rc = e.returnCode();
    art::printCmsException(e, kProgramName); // , "Thing1", rc);
  }
  catch (cet::exception& e) {
    rc = 8001;
    art::printCmsException(e, kProgramName); // , "Thing2", rc);
  }
  catch(std::bad_alloc& bda) {
    rc = 8004;
    art::printBadAllocException(kProgramName); // , "Thing3", rc);
  }
  catch (std::exception& e) {
    rc = 8002;
    art::printStdException(e, kProgramName); // , "Thing4", rc);
  }
  catch (...) {
    rc = 8003;
    art::printUnknownException(kProgramName); // , "Thing5", rc);
  }
  // Disable Root Error Handler again, just in case an exception
  // caused the above disabling of the handler to be bypassed.
  SetErrorHandler(DefaultErrorHandler);
  return rc;
}
