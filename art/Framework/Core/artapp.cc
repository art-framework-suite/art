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
#include "art/ParameterSet/MakeParameterSets.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/ExceptionMessages.h"
#include "art/Utilities/Presence.h"
#include "art/Utilities/RootHandlers.h"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include "MessageFacility/MessageLogger.h"
#include "TError.h"
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


static char const* const kParameterSetOpt = "parameter-set";
static char const* const kPythonOpt = "pythonOptions";
static char const* const kParameterSetCommandOpt = "parameter-set,p";
static char const* const kJobreportCommandOpt = "jobreport,j";
static char const* const kEnableJobreportCommandOpt = "enablejobreport,e";
static char const* const kJobModeCommandOpt = "mode,m";
static char const* const kMultiThreadMessageLoggerOpt = "multithreadML,t";
static char const* const kHelpOpt = "help";
static char const* const kHelpCommandOpt = "help,h";
static char const* const kStrictOpt = "strict";
static char const* const kProgramName = "art";

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
        catch (artZ::Exception& e) {
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

  // We must initialize the plug-in manager first
  try {
    artplugin::PluginManager::configure(artplugin::standard::config());
  }
  catch(artZ::Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // Decide whether to use the multi-thread or single-thread message logger
  //    (Just walk the command-line arguments, since the boost parser will
  //    be run below and can lead to error messages which should be sent via
  //    the message logger)
  bool multiThreadML = false;
  for (int i=0; i<argc; ++i) {
    if ( (std::strncmp (argv[i],"-t", 20) == 0) ||
         (std::strncmp (argv[i],"--multithreadML", 20) == 0) )
    { multiThreadML = true;
      break;
    }
  }

  // Load the message service plug-in
  boost::shared_ptr<art::Presence> theMessageServicePresence;

  if (multiThreadML)
  {
    try {
      theMessageServicePresence = boost::shared_ptr<art::Presence>(art::PresenceFactory::get()->
          makePresence("MessageServicePresence").release());
    }
    catch(artZ::Exception& e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  } else {
    try {
      theMessageServicePresence = boost::shared_ptr<art::Presence>(art::PresenceFactory::get()->
          makePresence("SingleThreadMSPresence").release());
    }
    catch(artZ::Exception& e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }

  //
  // Specify default services to be enabled with their default parameters.
  //
  // The parameters for these can be overridden from the configuration files.
  std::vector<std::string> defaultServices;
  defaultServices.reserve(5);
  defaultServices.push_back("MessageLogger");
  defaultServices.push_back("InitRootHandlers");
  // defaultServices.push_back("AdaptorConfig");
  defaultServices.push_back("EnableFloatingPointExceptions");
  defaultServices.push_back("UnixSignalService");

  // These cannot be overridden from the configuration files.
  // An exception will be thrown if any of these is specified there.
  std::vector<std::string> forcedServices;
  //forcedServices.reserve(2);
  //forcedServices.push_back("JobReportService");
  //forcedServices.push_back("SiteLocalConfigService");
  forcedServices.push_back("CurrentModuleService");

  std::string descString(argv[0]);
  descString += " [options] [--";
  descString += kParameterSetOpt;
  descString += "] config_file \nAllowed options";
  boost::program_options::options_description desc(descString);

  desc.add_options()
    (kHelpCommandOpt, "produce help message")
    (kParameterSetCommandOpt, boost::program_options::value<std::string>(), "configuration file")
    (kJobreportCommandOpt, boost::program_options::value<std::string>(),
        "file name to use for a job report file: default extension is .xml")
    (kEnableJobreportCommandOpt,
        "enable job report files (if any) specified in configuration file")
    (kJobModeCommandOpt, boost::program_options::value<std::string>(),
        "Job Mode for MessageLogger defaults - default mode is grid")
    (kMultiThreadMessageLoggerOpt,
        "MessageLogger handles multiple threads - default is single-thread")
    (kStrictOpt, "strict parsing");

  // anything at the end will be ignored, and sent to python
  boost::program_options::positional_options_description p;
  p.add(kParameterSetOpt, 1).add(kPythonOpt, -1);

  // This --fwk option is not used anymore, but I'm leaving it around as
  // it might be useful again in the future for code development
  // purposes.  We originally used it when implementing the boost
  // state machine code.
  boost::program_options::options_description hidden("hidden options");
  hidden.add_options()("fwk", "For use only by Framework Developers")
    (kPythonOpt, boost::program_options::value< std::vector<std::string> >(),
     "options at the end to be passed to python");

  boost::program_options::options_description all_options("All Options");
  all_options.add(desc).add(hidden);

  boost::program_options::variables_map vm;
  try {
    store(boost::program_options::command_line_parser(argc,argv).options(all_options).positional(p).run(),vm);
    notify(vm);
  }
  catch(boost::program_options::error const& iException) {
    mf::LogError("FwkJob")
      << "Exception from command line processing: " << iException.what();
    mf::LogSystem("CommandLineProcessing")
      << "Exception from command line processing: " << iException.what() << "\n";
    return 7000;
  }

  if(vm.count(kHelpOpt)) {
    std::cout << desc <<std::endl;
    if(!vm.count(kParameterSetOpt)) mf::HaltMessageLogging();
    return 0;
  }

  if(!vm.count(kParameterSetOpt)) {
    std::string shortDesc("ConfigFileNotFound");
    std::ostringstream longDesc;
    longDesc << "art: No configuration file given.\n"
                "For usage and an options list, please do '"
             << argv[0] <<  " --" << kHelpOpt
             << "'.";
    int exitCode = 7001;
    mf::LogAbsolute(shortDesc) << longDesc.str() << "\n";
    mf::HaltMessageLogging();
    return exitCode;
  }

  //
  // Decide whether to enable creation of job report xml file
  //  We do this first so any errors will be reported
  //
  std::auto_ptr<std::ofstream> jobReportStreamPtr;
  if (vm.count("jobreport")) {
    std::string jobReportFile = vm["jobreport"].as<std::string>();
    jobReportStreamPtr = std::auto_ptr<std::ofstream>( new std::ofstream(jobReportFile.c_str()) );
  } else if (vm.count("enablejobreport")) {
    jobReportStreamPtr = std::auto_ptr<std::ofstream>( new std::ofstream("FrameworkJobReport.xml") );
  }
  //
  // Make JobReport Service up front
  //
  //NOTE: JobReport must have a lifetime shorter than jobReportStreamPtr so that when the JobReport destructor
  // is called jobReportStreamPtr is still valid

  art::ServiceToken jobReportToken;

  std::string fileName(vm[kParameterSetOpt].as<std::string>());
  boost::shared_ptr<art::ProcessDesc> processDesc;
  try {
    processDesc = art::readConfig(fileName, argc, argv);
  }
  catch(artZ::Exception& iException) {
    std::string shortDesc("ConfigFileReadError");
    std::ostringstream longDesc;
    longDesc << "Problem with configuration file " << fileName
             <<  "\n" << iException.what();
    int exitCode = 7002;
    mf::LogSystem(shortDesc) << longDesc.str() << "\n";
    return exitCode;
  }

  processDesc->addServices(defaultServices, forcedServices);
  //
  // Decide what mode of hardcoded MessageLogger defaults to use
  //
  if (vm.count("mode")) {
    std::string jobMode = vm["mode"].as<std::string>();
    mf::MessageDrop::instance()->jobMode = jobMode;
  }

  if(vm.count(kStrictOpt))
  {
    //art::setStrictParsing(true);
    mf::LogSystem("CommandLineProcessing")
      << "Strict configuration processing is now done from python";
  }

  // Now create and configure the services
  //
  EventProcessorWithSentry proc;
  int rc = -1; // we should never return this value!
  try {
    std::auto_ptr<art::EventProcessor>
        procP(new
              art::EventProcessor(processDesc, jobReportToken,
                             art::serviceregistry::kTokenOverrides));
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
  catch (artZ::Exception& e) {
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
