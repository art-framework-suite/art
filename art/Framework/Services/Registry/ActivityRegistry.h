#ifndef FWCore_ServiceRegistry_ActivityRegistry_h
#define FWCore_ServiceRegistry_ActivityRegistry_h

//
// Package:     ServiceRegistry
// Class  :     ActivityRegistry
//
/**\class ActivityRegistry ActivityRegistry.h FWCore/ServiceRegistry/interface/ActivityRegistry.h

 Description: Registry holding the signals that Services can subscribe to

 Usage:
   Services can connect to the signals distributed by the ActivityRegistry in order to monitor the activity of the application.

*/


// system include files
#include "boost/bind.hpp"
#include "boost/mem_fn.hpp"
#include "boost/utility.hpp"
#include "sigc++/signal.h"


// forward declarations
namespace edm {
  class Event;
  class EventID;
  class HLTPathStatus;
  class SubRun;
  class SubRunID;
  class ModuleDescription;
  class Run;
  class RunID;
  class Timestamp;
}  // namespace edm

// helper macros
#if defined(AR_WATCH_VIA_0_ARG_METHOD) \
 || defined(AR_WATCH_VIA_1_ARG_METHOD) \
 || defined(AR_WATCH_VIA_2_ARG_METHOD)
  #error "ActivityRegistry: AR_WATCH_VIA_x_ARG_METHOD already #defined!"
#else
  #define AR_WATCH_VIA_0_ARG_METHOD(method) template<class TClass, class TMethod> void method (TClass* iObject, TMethod iMethod) { method (boost::bind(boost::mem_fn(iMethod), iObject)); }
  #define AR_WATCH_VIA_1_ARG_METHOD(method) template<class TClass, class TMethod> void method (TClass* iObject, TMethod iMethod) { method (boost::bind(boost::mem_fn(iMethod), iObject, _1)); }
  #define AR_WATCH_VIA_2_ARG_METHOD(method) template<class TClass, class TMethod> void method (TClass* iObject, TMethod iMethod) { method (boost::bind(boost::mem_fn(iMethod), iObject, _1,_2)); }
#endif  // #if

namespace edm {

  struct ActivityRegistry : private boost::noncopyable
  {
    ActivityRegistry() {}

    // ---------- signals ------------------------------------
    typedef sigc::signal<void> PostBeginJob;
    ///signal is emitted after all modules have gotten their beginJob called
    PostBeginJob postBeginJobSignal_;
    ///convenience function for attaching to signal
    void watchPostBeginJob(PostBeginJob::slot_type const& iSlot) {
      postBeginJobSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostBeginJob)

    typedef sigc::signal<void> PostEndJob;
    ///signal is emitted after all modules have gotten their endJob called
    PostEndJob postEndJobSignal_;
    void watchPostEndJob(PostEndJob::slot_type const& iSlot) {
      PostEndJob::slot_list_type sl = postEndJobSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostEndJob)

    typedef sigc::signal<void> JobFailure;
    /// signal is emitted if event processing or end-of-job
    /// processing fails with an uncaught exception.
    JobFailure    jobFailureSignal_;
    ///convenience function for attaching to signal
    void watchJobFailure(JobFailure::slot_type const& iSlot) {
      JobFailure::slot_list_type sl = jobFailureSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchJobFailure)

    /// signal is emitted before the source starts creating an Event
    typedef sigc::signal<void> PreSource;
    PreSource preSourceSignal_;
    void watchPreSource(PreSource::slot_type const& iSlot) {
      preSourceSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPreSource)

    /// signal is emitted after the source starts creating an Event
    typedef sigc::signal<void> PostSource;
    PostSource postSourceSignal_;
    void watchPostSource(PostSource::slot_type const& iSlot) {
      PostSource::slot_list_type sl = postSourceSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostSource)

    /// signal is emitted before the source starts creating a SubRun
    typedef sigc::signal<void> PreSourceSubRun;
    PreSourceSubRun preSourceLumiSignal_;
    void watchPreSourceSubRun(PreSourceSubRun::slot_type const& iSlot) {
      preSourceLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPreSourceSubRun)

    /// signal is emitted after the source starts creating a SubRun
    typedef sigc::signal<void> PostSourceSubRun;
    PostSourceSubRun postSourceLumiSignal_;
    void watchPostSourceSubRun(PostSourceSubRun::slot_type const& iSlot) {
      PostSourceSubRun::slot_list_type sl = postSourceLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostSourceSubRun)

    /// signal is emitted before the source starts creating a Run
    typedef sigc::signal<void> PreSourceRun;
    PreSourceRun preSourceRunSignal_;
    void watchPreSourceRun(PreSourceRun::slot_type const& iSlot) {
      preSourceRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPreSourceRun)

    /// signal is emitted after the source starts creating a Run
    typedef sigc::signal<void> PostSourceRun;
    PostSourceRun postSourceRunSignal_;
    void watchPostSourceRun(PostSourceRun::slot_type const& iSlot) {
      PostSourceRun::slot_list_type sl = postSourceRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostSourceRun)

    /// signal is emitted before the source opens a file
    typedef sigc::signal<void> PreOpenFile;
    PreOpenFile preOpenFileSignal_;
    void watchPreOpenFile(PreOpenFile::slot_type const& iSlot) {
      preOpenFileSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPreOpenFile)

    /// signal is emitted after the source opens a file
    typedef sigc::signal<void> PostOpenFile;
    PostOpenFile postOpenFileSignal_;
    void watchPostOpenFile(PostOpenFile::slot_type const& iSlot) {
      PostOpenFile::slot_list_type sl = postOpenFileSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostOpenFile)

    /// signal is emitted before the Closesource closes a file
    typedef sigc::signal<void> PreCloseFile;
    PreCloseFile preCloseFileSignal_;
    void watchPreCloseFile(PreCloseFile::slot_type const& iSlot) {
      preCloseFileSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPreCloseFile)

    /// signal is emitted after the source opens a file
    typedef sigc::signal<void> PostCloseFile;
    PostCloseFile postCloseFileSignal_;
    void watchPostCloseFile(PostCloseFile::slot_type const& iSlot) {
      PostCloseFile::slot_list_type sl = postCloseFileSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_0_ARG_METHOD(watchPostCloseFile)

    typedef sigc::signal<void, edm::EventID const&, edm::Timestamp const&> PreProcessEvent;
    /// signal is emitted after the Event has been created by the InputSource but before any modules have seen the Event
    PreProcessEvent preProcessEventSignal_;
    void watchPreProcessEvent(PreProcessEvent::slot_type const& iSlot) {
      preProcessEventSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPreProcessEvent)

    typedef sigc::signal<void, Event const&> PostProcessEvent;
    /// signal is emitted after all modules have finished processing the Event
    PostProcessEvent postProcessEventSignal_;
    void watchPostProcessEvent(PostProcessEvent::slot_type const& iSlot) {
      PostProcessEvent::slot_list_type sl = postProcessEventSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostProcessEvent)

    typedef sigc::signal<void, edm::RunID const&, edm::Timestamp const&> PreBeginRun;
    /// signal is emitted after the Run has been created by the InputSource but before any modules have seen the Run
    PreBeginRun preBeginRunSignal_;
    void watchPreBeginRun(PreBeginRun::slot_type const& iSlot) {
      preBeginRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPreBeginRun)

    typedef sigc::signal<void, Run const&> PostBeginRun;
    /// signal is emitted after all modules have finished processing the beginRun
    PostBeginRun postBeginRunSignal_;
    void watchPostBeginRun(PostBeginRun::slot_type const& iSlot) {
      PostBeginRun::slot_list_type sl = postBeginRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostBeginRun)

    typedef sigc::signal<void, edm::RunID const&, edm::Timestamp const&> PreEndRun;
    /// signal is emitted before the endRun is processed
    PreEndRun preEndRunSignal_;
    void watchPreEndRun(PreEndRun::slot_type const& iSlot) {
      preEndRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPreEndRun)

    typedef sigc::signal<void, Run const&> PostEndRun;
    /// signal is emitted after all modules have finished processing the Run
    PostEndRun postEndRunSignal_;
    void watchPostEndRun(PostEndRun::slot_type const& iSlot) {
      PostEndRun::slot_list_type sl = postEndRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostEndRun)

    typedef sigc::signal<void, edm::SubRunID const&, edm::Timestamp const&> PreBeginSubRun;
    /// signal is emitted after the SubRun has been created by the InputSource but before any modules have seen the SubRun
    PreBeginSubRun preBeginLumiSignal_;
    void watchPreBeginSubRun(PreBeginSubRun::slot_type const& iSlot) {
      preBeginLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPreBeginSubRun)

    typedef sigc::signal<void, SubRun const&> PostBeginSubRun;
    /// signal is emitted after all modules have finished processing the beginLumi
    PostBeginSubRun postBeginLumiSignal_;
    void watchPostBeginSubRun(PostBeginSubRun::slot_type const& iSlot) {
      PostBeginSubRun::slot_list_type sl = postBeginLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostBeginSubRun)

    typedef sigc::signal<void, edm::SubRunID const&, edm::Timestamp const&> PreEndSubRun;
    /// signal is emitted before the endLumi is processed
    PreEndSubRun preEndLumiSignal_;
    void watchPreEndSubRun(PreEndSubRun::slot_type const& iSlot) {
      preEndLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPreEndSubRun)

    typedef sigc::signal<void, SubRun const&> PostEndSubRun;
    /// signal is emitted after all modules have finished processing the SubRun
    PostEndSubRun postEndLumiSignal_;
    void watchPostEndSubRun(PostEndSubRun::slot_type const& iSlot) {
      PostEndSubRun::slot_list_type sl = postEndLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostEndSubRun)

    /// signal is emitted before starting to process a Path for an event
    typedef sigc::signal<void, std::string const&> PreProcessPath;
    PreProcessPath preProcessPathSignal_;
    void watchPreProcessPath(PreProcessPath::slot_type const& iSlot) {
      preProcessPathSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreProcessPath)

    /// signal is emitted after all modules have finished for the Path for an event
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostProcessPath;
    PostProcessPath postProcessPathSignal_;
    void watchPostProcessPath(PostProcessPath::slot_type const& iSlot) {
      PostProcessPath::slot_list_type sl = postProcessPathSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostProcessPath)

    /// signal is emitted before starting to process a Path for beginRun
    typedef sigc::signal<void, std::string const&> PrePathBeginRun;
    PrePathBeginRun prePathBeginRunSignal_;
    void watchPrePathBeginRun(PrePathBeginRun::slot_type const& iSlot) {
      prePathBeginRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPrePathBeginRun)

    /// signal is emitted after all modules have finished for the Path for beginRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathBeginRun;
    PostPathBeginRun postPathBeginRunSignal_;
    void watchPostPathBeginRun(PostPathBeginRun::slot_type const& iSlot) {
      PostPathBeginRun::slot_list_type sl = postPathBeginRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostPathBeginRun)

    /// signal is emitted before starting to process a Path for endRun
    typedef sigc::signal<void, std::string const&> PrePathEndRun;
    PrePathEndRun prePathEndRunSignal_;
    void watchPrePathEndRun(PrePathEndRun::slot_type const& iSlot) {
      prePathEndRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPrePathEndRun)

    /// signal is emitted after all modules have finished for the Path for endRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathEndRun;
    PostPathEndRun postPathEndRunSignal_;
    void watchPostPathEndRun(PostPathEndRun::slot_type const& iSlot) {
      PostPathEndRun::slot_list_type sl = postPathEndRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostPathEndRun)

    /// signal is emitted before starting to process a Path for beginLumi
    typedef sigc::signal<void, std::string const&> PrePathBeginSubRun;
    PrePathBeginSubRun prePathBeginLumiSignal_;
    void watchPrePathBeginSubRun(PrePathBeginSubRun::slot_type const& iSlot) {
      prePathBeginLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPrePathBeginSubRun)

    /// signal is emitted after all modules have finished for the Path for beginLumi
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathBeginSubRun;
    PostPathBeginSubRun postPathBeginLumiSignal_;
    void watchPostPathBeginSubRun(PostPathBeginSubRun::slot_type const& iSlot) {
      PostPathBeginSubRun::slot_list_type sl = postPathBeginLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostPathBeginSubRun)

    /// signal is emitted before starting to process a Path for endRun
    typedef sigc::signal<void, std::string const&> PrePathEndSubRun;
    PrePathEndSubRun prePathEndLumiSignal_;
    void watchPrePathEndSubRun(PrePathEndSubRun::slot_type const& iSlot) {
      prePathEndLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPrePathEndSubRun)

    /// signal is emitted after all modules have finished for the Path for endRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathEndSubRun;
    PostPathEndSubRun postPathEndLumiSignal_;
    void watchPostPathEndSubRun(PostPathEndSubRun::slot_type const& iSlot) {
      PostPathEndSubRun::slot_list_type sl = postPathEndLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_2_ARG_METHOD(watchPostPathEndSubRun)

    /// signal is emitted before the module is constructed
    typedef sigc::signal<void, ModuleDescription const&> PreModuleConstruction;
    PreModuleConstruction preModuleConstructionSignal_;
    void watchPreModuleConstruction(PreModuleConstruction::slot_type const& iSlot) {
      preModuleConstructionSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleConstruction)

    /// signal is emitted after the module was construction
    typedef sigc::signal<void, ModuleDescription const&> PostModuleConstruction;
    PostModuleConstruction postModuleConstructionSignal_;
    void watchPostModuleConstruction(PostModuleConstruction::slot_type const& iSlot) {
      PostModuleConstruction::slot_list_type sl = postModuleConstructionSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleConstruction)

    /// signal is emitted before the module does beginJob
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginJob;
    PreModuleBeginJob preModuleBeginJobSignal_;
    void watchPreModuleBeginJob(PreModuleBeginJob::slot_type const& iSlot) {
      preModuleBeginJobSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleBeginJob)

    /// signal is emitted after the module had done beginJob
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginJob;
    PostModuleBeginJob postModuleBeginJobSignal_;
    void watchPostModuleBeginJob(PostModuleBeginJob::slot_type const& iSlot) {
      PostModuleBeginJob::slot_list_type sl = postModuleBeginJobSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleBeginJob)

    /// signal is emitted before the module does endJob
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndJob;
    PreModuleEndJob preModuleEndJobSignal_;
    void watchPreModuleEndJob(PreModuleEndJob::slot_type const& iSlot) {
      preModuleEndJobSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleEndJob)

    /// signal is emitted after the module had done endJob
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndJob;
    PostModuleEndJob postModuleEndJobSignal_;
    void watchPostModuleEndJob(PostModuleEndJob::slot_type const& iSlot) {
      PostModuleEndJob::slot_list_type sl = postModuleEndJobSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleEndJob)

    /// signal is emitted before the module starts processing the Event
    typedef sigc::signal<void, ModuleDescription const&> PreModule;
    PreModule preModuleSignal_;
    void watchPreModule(PreModule::slot_type const& iSlot) {
      preModuleSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModule)

    /// signal is emitted after the module finished processing the Event
    typedef sigc::signal<void, ModuleDescription const&> PostModule;
    PostModule postModuleSignal_;
    void watchPostModule(PostModule::slot_type const& iSlot) {
      PostModule::slot_list_type sl = postModuleSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModule)

    /// signal is emitted before the module starts processing beginRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginRun;
    PreModuleBeginRun preModuleBeginRunSignal_;
    void watchPreModuleBeginRun(PreModuleBeginRun::slot_type const& iSlot) {
      preModuleBeginRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleBeginRun)

    /// signal is emitted after the module finished processing beginRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginRun;
    PostModuleBeginRun postModuleBeginRunSignal_;
    void watchPostModuleBeginRun(PostModuleBeginRun::slot_type const& iSlot) {
      PostModuleBeginRun::slot_list_type sl = postModuleBeginRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleBeginRun)

    /// signal is emitted before the module starts processing endRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndRun;
    PreModuleEndRun preModuleEndRunSignal_;
    void watchPreModuleEndRun(PreModuleEndRun::slot_type const& iSlot) {
      preModuleEndRunSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleEndRun)

    /// signal is emitted after the module finished processing endRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndRun;
    PostModuleEndRun postModuleEndRunSignal_;
    void watchPostModuleEndRun(PostModuleEndRun::slot_type const& iSlot) {
      PostModuleEndRun::slot_list_type sl = postModuleEndRunSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleEndRun)

    /// signal is emitted before the module starts processing beginLumi
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginSubRun;
    PreModuleBeginSubRun preModuleBeginLumiSignal_;
    void watchPreModuleBeginSubRun(PreModuleBeginSubRun::slot_type const& iSlot) {
      preModuleBeginLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleBeginSubRun)

    /// signal is emitted after the module finished processing beginLumi
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginSubRun;
    PostModuleBeginSubRun postModuleBeginLumiSignal_;
    void watchPostModuleBeginSubRun(PostModuleBeginSubRun::slot_type const& iSlot) {
      PostModuleBeginSubRun::slot_list_type sl = postModuleBeginLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleBeginSubRun)

    /// signal is emitted before the module starts processing endLumi
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndSubRun;
    PreModuleEndSubRun preModuleEndLumiSignal_;
    void watchPreModuleEndSubRun(PreModuleEndSubRun::slot_type const& iSlot) {
      preModuleEndLumiSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreModuleEndSubRun)

    /// signal is emitted after the module finished processing endLumi
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndSubRun;
    PostModuleEndSubRun postModuleEndLumiSignal_;
    void watchPostModuleEndSubRun(PostModuleEndSubRun::slot_type const& iSlot) {
      PostModuleEndSubRun::slot_list_type sl = postModuleEndLumiSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostModuleEndSubRun)

    /// signal is emitted before the source is constructed
    typedef sigc::signal<void, ModuleDescription const&> PreSourceConstruction;
    PreSourceConstruction preSourceConstructionSignal_;
    void watchPreSourceConstruction(PreSourceConstruction::slot_type const& iSlot) {
      preSourceConstructionSignal_.connect(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPreSourceConstruction)

    /// signal is emitted after the source was construction
    typedef sigc::signal<void, ModuleDescription const&> PostSourceConstruction;
    PostSourceConstruction postSourceConstructionSignal_;
    void watchPostSourceConstruction(PostSourceConstruction::slot_type const& iSlot) {
      PostSourceConstruction::slot_list_type sl = postSourceConstructionSignal_.slots();
      sl.push_front(iSlot);
    }
    AR_WATCH_VIA_1_ARG_METHOD(watchPostSourceConstruction)
      // ---------- member functions ---------------------------

    ///forwards our signals to slots connected to iOther
    void connect(ActivityRegistry& iOther);

    ///copy the slots from iOther and connect them directly to our own
    /// this allows us to 'forward' signals more efficiently,
    /// BUT if iOther gains new slots after this call, we will not see them
    /// This is also careful to keep the order of the slots proper
    /// for services.
    void copySlotsFrom(ActivityRegistry& iOther);

  private:
  };  // ActivityRegistry

}  // namespace edm

#undef AR_WATCH_VIA_0_ARG_METHOD
#undef AR_WATCH_VIA_1_ARG_METHOD
#undef AR_WATCH_VIA_2_ARG_METHOD

#if defined(AR_WATCH_VIA_0_ARG_METHOD) \
 || defined(AR_WATCH_VIA_1_ARG_METHOD) \
 || defined(AR_WATCH_VIA_2_ARG_METHOD)
  #error "ActivityRegistry: AR_WATCH_VIA_x_ARG_METHOD is still #defined!"
#endif  // #if

#endif  // FWCore_ServiceRegistry_ActivityRegistry_h
