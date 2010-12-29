// ======================================================================
//
// This is a generic main that can be used with any plugin and a PSet
// script.   See notes in EventProcessor.cpp for details about it.
//
// ======================================================================

#include "TError.h"
#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/standard.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/RootHandlers.h"
#include "boost/program_options.hpp"
#include "boost/shared_ptr.hpp"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
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
          << "--config config_file";

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
              << ": " << e.what();
    return 7000;
  }

  if (vm.count("Help")) {
    std::cout << desc <<std::endl;
    return 0;
  }

  if (!vm.count("parameter-set")) {
    std::cerr << "Exception from command line processing in " << argv[0]
              << ": no configuration file given.\n"
              << "For usage and an options list, please do '"
              << argv[0] <<  " --help"
              << "'.";
    return 7001;
  }

  // ------------------
  //
  // Get the parameter set from parsing the configuration file.
  //
  fhicl::ParameterSet main_pset, ancillary_pset;
  fhicl::intermediate_table raw_config;
  string config_filename = vm["config"].as<string>();
  ifstream config_stream(config_filename.c_str());
  if (!fhicl::parse_document(config_stream, raw_config))
    {
      std::cerr << "Failed to parse the configuration file '"
                << config_filename
                << "'\n";
      return 7001;
    }

  //
  // Start the messagefacility
  //
#if 0

#if 0
  mf::start_me(multithread,
               ancillary_pset.get<fhicl::ParameterSet>("message_facility"));
#endif
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
  try {
    std::auto_ptr<art::EventProcessor> procP;
#if 0
        procP(new
              art::EventProcessor(processDesc, jobReportToken,
                             art::kTokenOverrides));
#endif
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
    art::ServiceHandle<art::RootHandlers> rootHandler;
    rootHandler->disableErrorHandler();
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

  // Disable Root Error Handler again, just in case an exception
  // caused the above disabling of the handler to be bypassed.
  SetErrorHandler(DefaultErrorHandler);
#endif
  return rc;
}  // artapp
