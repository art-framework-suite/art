#ifndef art_Framework_Services_Registry_ActivityRegistry_h
#define art_Framework_Services_Registry_ActivityRegistry_h

// ======================================================================
//
// ActivityRegistry - Registry holding the signals that Services can
//                    subscribe to
//
// Services can connect to the signals distributed by the
// ActivityRegistry in order to monitor the activity of the application.
//
// ======================================================================

#include "cpp0x/functional"
#include "sigc++/signal.h"
#include <string>
#include <vector>

namespace art {
  // Explicit declarations here rather than including fwd.h to avoid
  // confusing our dependency checker into thinking there's a link
  // dependency.
  class Event;
  class HLTPathStatus;
  class InputSource;
  class SubRun;
  class SubRunID;
  class ModuleDescription;
  class Run;
  class RunID;
  class Timestamp;
  class Worker;

  class ActivityRegistry;
}  // art

// Helper macros
#define AR_DECL_STATE_0_ARG_FUNC(stateTag)                      \
  template<class TClass, class TMethod>                         \
  void                                                          \
  watch##stateTag (TClass* iObject, TMethod iMethod) {          \
    watch##stateTag (std::bind(std::mem_fn(iMethod), iObject)); \
  }
#define AR_DECL_STATE_1_ARG_FUNC(stateTag)                          \
  template<class TClass, class TMethod>                             \
  void                                                              \
  watch##stateTag (TClass* iObject, TMethod iMethod) {              \
    watch##stateTag (std::bind(std::mem_fn(iMethod), iObject, std::placeholders::_1)); \
  }
#define AR_DECL_STATE_2_ARG_FUNC(stateTag)                              \
  template<class TClass, class TMethod>                                 \
  void                                                                  \
  watch##stateTag (TClass* iObject, TMethod iMethod) {                  \
    watch##stateTag (std::bind(std::mem_fn(iMethod), iObject, std::placeholders::_1,std::placeholders::_2));  \
  }

class art::ActivityRegistry {
public:
  ActivityRegistry()
#ifndef __GCCXML__
 = default;
  ActivityRegistry(ActivityRegistry const&) = delete;
  ActivityRegistry& operator=(ActivityRegistry const&) = delete;
#else
  ;
#endif

  // ---------- signals ------------------------------------
  typedef sigc::signal<void> PostBeginJob;
  // Signal is emitted after all modules have gotten their beginJob called
  PostBeginJob postBeginJobSignal_;
  // Convenience function for attaching to signal
  void watchPostBeginJob(PostBeginJob::slot_type const& iSlot) {
    postBeginJobSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostBeginJob)

    typedef sigc::signal<void> PostEndJob;
  ///signal is emitted after all modules have gotten their endJob called
  PostEndJob postEndJobSignal_;
  void watchPostEndJob(PostEndJob::slot_type const& iSlot) {
    PostEndJob::slot_list_type sl = postEndJobSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostEndJob)

    typedef sigc::signal<void> JobFailure;
  // Signal is emitted if event processing or end-of-job
  // processing fails with an uncaught exception.
  JobFailure    jobFailureSignal_;
  // Convenience function for attaching to signal
  void watchJobFailure(JobFailure::slot_type const& iSlot) {
    JobFailure::slot_list_type sl = jobFailureSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(JobFailure)

  // Signal is emitted before the source starts creating an Event
    typedef sigc::signal<void> PreSource;
  PreSource preSourceSignal_;
  void watchPreSource(PreSource::slot_type const& iSlot) {
    preSourceSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PreSource)

  // Signal is emitted after the source starts creating an Event
    typedef sigc::signal<void> PostSource;
  PostSource postSourceSignal_;
  void watchPostSource(PostSource::slot_type const& iSlot) {
    PostSource::slot_list_type sl = postSourceSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostSource)

  // Signal is emitted before the source starts creating a SubRun
    typedef sigc::signal<void> PreSourceSubRun;
  PreSourceSubRun preSourceSubRunSignal_;
  void watchPreSourceSubRun(PreSourceSubRun::slot_type const& iSlot) {
    preSourceSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PreSourceSubRun)

  // Signal is emitted after the source starts creating a SubRun
    typedef sigc::signal<void> PostSourceSubRun;
  PostSourceSubRun postSourceSubRunSignal_;
  void watchPostSourceSubRun(PostSourceSubRun::slot_type const& iSlot) {
    PostSourceSubRun::slot_list_type sl = postSourceSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostSourceSubRun)

  // Signal is emitted before the source starts creating a Run
    typedef sigc::signal<void> PreSourceRun;
  PreSourceRun preSourceRunSignal_;
  void watchPreSourceRun(PreSourceRun::slot_type const& iSlot) {
    preSourceRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PreSourceRun)

  // Signal is emitted after the source starts creating a Run
    typedef sigc::signal<void> PostSourceRun;
  PostSourceRun postSourceRunSignal_;
  void watchPostSourceRun(PostSourceRun::slot_type const& iSlot) {
    PostSourceRun::slot_list_type sl = postSourceRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostSourceRun)

  // Signal is emitted before the source opens a file
    typedef sigc::signal<void> PreOpenFile;
  PreOpenFile preOpenFileSignal_;
  void watchPreOpenFile(PreOpenFile::slot_type const& iSlot) {
    preOpenFileSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PreOpenFile)

  // Signal is emitted after the source opens a file
    typedef sigc::signal<void, std::string const &> PostOpenFile;
  PostOpenFile postOpenFileSignal_;
  void watchPostOpenFile(PostOpenFile::slot_type const& iSlot) {
    PostOpenFile::slot_list_type sl = postOpenFileSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostOpenFile)

  // Signal is emitted before the Closesource closes a file
    typedef sigc::signal<void> PreCloseFile;
  PreCloseFile preCloseFileSignal_;
  void watchPreCloseFile(PreCloseFile::slot_type const& iSlot) {
    preCloseFileSignal_.connect(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PreCloseFile)

  // Signal is emitted after the source opens a file
    typedef sigc::signal<void> PostCloseFile;
  PostCloseFile postCloseFileSignal_;
  void watchPostCloseFile(PostCloseFile::slot_type const& iSlot) {
    PostCloseFile::slot_list_type sl = postCloseFileSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_0_ARG_FUNC(PostCloseFile)

    typedef sigc::signal<void, Event const&> PreProcessEvent;
  // Signal is emitted after the Event has been created by the
  // InputSource but before any modules have seen the Event
  PreProcessEvent preProcessEventSignal_;
  void watchPreProcessEvent(PreProcessEvent::slot_type const& iSlot) {
    preProcessEventSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreProcessEvent)

    typedef sigc::signal<void, Event const&> PostProcessEvent;
  // Signal is emitted after all modules have finished processing the Event
  PostProcessEvent postProcessEventSignal_;
  void watchPostProcessEvent(PostProcessEvent::slot_type const& iSlot) {
    PostProcessEvent::slot_list_type sl = postProcessEventSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostProcessEvent)

    typedef sigc::signal<void, Run const&> PreBeginRun;
  // Signal is emitted after the Run has been created by the InputSource but before any modules have seen the Run
  PreBeginRun preBeginRunSignal_;
  void watchPreBeginRun(PreBeginRun::slot_type const& iSlot) {
    preBeginRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreBeginRun)

    typedef sigc::signal<void, Run const&> PostBeginRun;
  // Signal is emitted after all modules have finished processing the beginRun
  PostBeginRun postBeginRunSignal_;
  void watchPostBeginRun(PostBeginRun::slot_type const& iSlot) {
    PostBeginRun::slot_list_type sl = postBeginRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostBeginRun)

    typedef sigc::signal<void, art::RunID const&, art::Timestamp const&> PreEndRun;
  // Signal is emitted before the endRun is processed
  PreEndRun preEndRunSignal_;
  void watchPreEndRun(PreEndRun::slot_type const& iSlot) {
    preEndRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PreEndRun)

    typedef sigc::signal<void, Run const&> PostEndRun;
  // Signal is emitted after all modules have finished processing the Run
  PostEndRun postEndRunSignal_;
  void watchPostEndRun(PostEndRun::slot_type const& iSlot) {
    PostEndRun::slot_list_type sl = postEndRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostEndRun)

    typedef sigc::signal<void, art::SubRun const&> PreBeginSubRun;
  // Signal is emitted after the SubRun has been created by the InputSource but before any modules have seen the SubRun
  PreBeginSubRun preBeginSubRunSignal_;
  void watchPreBeginSubRun(PreBeginSubRun::slot_type const& iSlot) {
    preBeginSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreBeginSubRun)

    typedef sigc::signal<void, SubRun const&> PostBeginSubRun;
  // Signal is emitted after all modules have finished processing the beginSubRun
  PostBeginSubRun postBeginSubRunSignal_;
  void watchPostBeginSubRun(PostBeginSubRun::slot_type const& iSlot) {
    PostBeginSubRun::slot_list_type sl = postBeginSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostBeginSubRun)

    typedef sigc::signal<void, art::SubRunID const&, art::Timestamp const&> PreEndSubRun;
  // Signal is emitted before the endSubRun is processed
  PreEndSubRun preEndSubRunSignal_;
  void watchPreEndSubRun(PreEndSubRun::slot_type const& iSlot) {
    preEndSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PreEndSubRun)

    typedef sigc::signal<void, SubRun const&> PostEndSubRun;
  // Signal is emitted after all modules have finished processing the SubRun
  PostEndSubRun postEndSubRunSignal_;
  void watchPostEndSubRun(PostEndSubRun::slot_type const& iSlot) {
    PostEndSubRun::slot_list_type sl = postEndSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostEndSubRun)

  // Signal is emitted before starting to process a Path for an event
    typedef sigc::signal<void, std::string const&> PreProcessPath;
  PreProcessPath preProcessPathSignal_;
  void watchPreProcessPath(PreProcessPath::slot_type const& iSlot) {
    preProcessPathSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreProcessPath)

  // Signal is emitted after all modules have finished for the Path for an event
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostProcessPath;
  PostProcessPath postProcessPathSignal_;
  void watchPostProcessPath(PostProcessPath::slot_type const& iSlot) {
    PostProcessPath::slot_list_type sl = postProcessPathSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostProcessPath)

  // Signal is emitted before starting to process a Path for beginRun
    typedef sigc::signal<void, std::string const&> PrePathBeginRun;
  PrePathBeginRun prePathBeginRunSignal_;
  void watchPrePathBeginRun(PrePathBeginRun::slot_type const& iSlot) {
    prePathBeginRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PrePathBeginRun)

  // Signal is emitted after all modules have finished for the Path for beginRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathBeginRun;
  PostPathBeginRun postPathBeginRunSignal_;
  void watchPostPathBeginRun(PostPathBeginRun::slot_type const& iSlot) {
    PostPathBeginRun::slot_list_type sl = postPathBeginRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostPathBeginRun)

  // Signal is emitted before starting to process a Path for endRun
    typedef sigc::signal<void, std::string const&> PrePathEndRun;
  PrePathEndRun prePathEndRunSignal_;
  void watchPrePathEndRun(PrePathEndRun::slot_type const& iSlot) {
    prePathEndRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PrePathEndRun)

  // Signal is emitted after all modules have finished for the Path for endRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathEndRun;
  PostPathEndRun postPathEndRunSignal_;
  void watchPostPathEndRun(PostPathEndRun::slot_type const& iSlot) {
    PostPathEndRun::slot_list_type sl = postPathEndRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostPathEndRun)

  // Signal is emitted before starting to process a Path for beginSubRun
    typedef sigc::signal<void, std::string const&> PrePathBeginSubRun;
  PrePathBeginSubRun prePathBeginSubRunSignal_;
  void watchPrePathBeginSubRun(PrePathBeginSubRun::slot_type const& iSlot) {
    prePathBeginSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PrePathBeginSubRun)

  // Signal is emitted after all modules have finished for the Path for beginSubRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathBeginSubRun;
  PostPathBeginSubRun postPathBeginSubRunSignal_;
  void watchPostPathBeginSubRun(PostPathBeginSubRun::slot_type const& iSlot) {
    PostPathBeginSubRun::slot_list_type sl = postPathBeginSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostPathBeginSubRun)

  // Signal is emitted before starting to process a Path for endRun
    typedef sigc::signal<void, std::string const&> PrePathEndSubRun;
  PrePathEndSubRun prePathEndSubRunSignal_;
  void watchPrePathEndSubRun(PrePathEndSubRun::slot_type const& iSlot) {
    prePathEndSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PrePathEndSubRun)

  // Signal is emitted after all modules have finished for the Path for endRun
    typedef sigc::signal<void, std::string const&, HLTPathStatus const&> PostPathEndSubRun;
  PostPathEndSubRun postPathEndSubRunSignal_;
  void watchPostPathEndSubRun(PostPathEndSubRun::slot_type const& iSlot) {
    PostPathEndSubRun::slot_list_type sl = postPathEndSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostPathEndSubRun)

  // Signal is emitted before the module is constructed
    typedef sigc::signal<void, ModuleDescription const&> PreModuleConstruction;
  PreModuleConstruction preModuleConstructionSignal_;
  void watchPreModuleConstruction(PreModuleConstruction::slot_type const& iSlot) {
    preModuleConstructionSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleConstruction)

  // Signal is emitted after the module was construction
    typedef sigc::signal<void, ModuleDescription const&> PostModuleConstruction;
  PostModuleConstruction postModuleConstructionSignal_;
  void watchPostModuleConstruction(PostModuleConstruction::slot_type const& iSlot) {
    PostModuleConstruction::slot_list_type sl = postModuleConstructionSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleConstruction)

  // JBK added
  // Signal is emitted after beginJob
    typedef sigc::signal<void, InputSource*,std::vector<Worker*> const&> PostBeginJobWorkers;
  PostBeginJobWorkers postBeginJobWorkersSignal_;
  void watchPostBeginJobWorkers(PostBeginJobWorkers::slot_type const& iSlot) {
    PostBeginJobWorkers::slot_list_type sl = postBeginJobWorkersSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_2_ARG_FUNC(PostBeginJobWorkers)
  // end JBK added

  // Signal is emitted before the module does beginJob
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginJob;
  PreModuleBeginJob preModuleBeginJobSignal_;
  void watchPreModuleBeginJob(PreModuleBeginJob::slot_type const& iSlot) {
    preModuleBeginJobSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleBeginJob)

  // Signal is emitted after the module had done beginJob
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginJob;
  PostModuleBeginJob postModuleBeginJobSignal_;
  void watchPostModuleBeginJob(PostModuleBeginJob::slot_type const& iSlot) {
    PostModuleBeginJob::slot_list_type sl = postModuleBeginJobSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleBeginJob)

  // Signal is emitted before the module does endJob
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndJob;
  PreModuleEndJob preModuleEndJobSignal_;
  void watchPreModuleEndJob(PreModuleEndJob::slot_type const& iSlot) {
    preModuleEndJobSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleEndJob)

  // Signal is emitted after the module had done endJob
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndJob;
  PostModuleEndJob postModuleEndJobSignal_;
  void watchPostModuleEndJob(PostModuleEndJob::slot_type const& iSlot) {
    PostModuleEndJob::slot_list_type sl = postModuleEndJobSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleEndJob)

  // Signal is emitted before the module starts processing the Event
    typedef sigc::signal<void, ModuleDescription const&> PreModule;
  PreModule preModuleSignal_;
  void watchPreModule(PreModule::slot_type const& iSlot) {
    preModuleSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModule)

  // Signal is emitted after the module finished processing the Event
    typedef sigc::signal<void, ModuleDescription const&> PostModule;
  PostModule postModuleSignal_;
  void watchPostModule(PostModule::slot_type const& iSlot) {
    PostModule::slot_list_type sl = postModuleSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModule)

  // Signal is emitted before the module starts processing beginRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginRun;
  PreModuleBeginRun preModuleBeginRunSignal_;
  void watchPreModuleBeginRun(PreModuleBeginRun::slot_type const& iSlot) {
    preModuleBeginRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleBeginRun)

  // Signal is emitted after the module finished processing beginRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginRun;
  PostModuleBeginRun postModuleBeginRunSignal_;
  void watchPostModuleBeginRun(PostModuleBeginRun::slot_type const& iSlot) {
    PostModuleBeginRun::slot_list_type sl = postModuleBeginRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleBeginRun)

  // Signal is emitted before the module starts processing endRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndRun;
  PreModuleEndRun preModuleEndRunSignal_;
  void watchPreModuleEndRun(PreModuleEndRun::slot_type const& iSlot) {
    preModuleEndRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleEndRun)

  // Signal is emitted after the module finished processing endRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndRun;
  PostModuleEndRun postModuleEndRunSignal_;
  void watchPostModuleEndRun(PostModuleEndRun::slot_type const& iSlot) {
    PostModuleEndRun::slot_list_type sl = postModuleEndRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleEndRun)

  // Signal is emitted before the module starts processing beginSubRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleBeginSubRun;
  PreModuleBeginSubRun preModuleBeginSubRunSignal_;
  void watchPreModuleBeginSubRun(PreModuleBeginSubRun::slot_type const& iSlot) {
    preModuleBeginSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleBeginSubRun)

  // Signal is emitted after the module finished processing beginSubRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleBeginSubRun;
  PostModuleBeginSubRun postModuleBeginSubRunSignal_;
  void watchPostModuleBeginSubRun(PostModuleBeginSubRun::slot_type const& iSlot) {
    PostModuleBeginSubRun::slot_list_type sl = postModuleBeginSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleBeginSubRun)

  // Signal is emitted before the module starts processing endSubRun
    typedef sigc::signal<void, ModuleDescription const&> PreModuleEndSubRun;
  PreModuleEndSubRun preModuleEndSubRunSignal_;
  void watchPreModuleEndSubRun(PreModuleEndSubRun::slot_type const& iSlot) {
    preModuleEndSubRunSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PreModuleEndSubRun)

  // Signal is emitted after the module finished processing endSubRun
    typedef sigc::signal<void, ModuleDescription const&> PostModuleEndSubRun;
  PostModuleEndSubRun postModuleEndSubRunSignal_;
  void watchPostModuleEndSubRun(PostModuleEndSubRun::slot_type const& iSlot) {
    PostModuleEndSubRun::slot_list_type sl = postModuleEndSubRunSignal_.slots();
    sl.push_front(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostModuleEndSubRun)

  // Signal emitted any time a service gets reconfigured.
    typedef sigc::signal<void, std::string const &> PostServiceReconfigure;
  PostServiceReconfigure postServiceReconfigureSignal_;
  void watchPostServiceReconfigure(PostServiceReconfigure::slot_type const & iSlot) {
    postServiceReconfigureSignal_.connect(iSlot);
  }
  AR_DECL_STATE_1_ARG_FUNC(PostServiceReconfigure)

  // ---------- member functions ---------------------------

  // Forwards our signals to slots connected to iOther
    void connect(ActivityRegistry& iOther);

  // Copy the slots from iOther and connect them directly to our own
  // this allows us to 'forward' signals more efficiently, BUT if iOther
  // gains new slots after this call, we will not see them This is also
  // careful to keep the order of the slots proper for services.
  void copySlotsFrom(ActivityRegistry& iOther);

};  // ActivityRegistry

#undef AR_DECL_STATE_0_ARG_FUNC
#undef AR_DECL_STATE_1_ARG_FUNC
#undef AR_DECL_STATE_2_ARG_FUNC

#endif /* art_Framework_Services_Registry_ActivityRegistry_h */

// Local Variables:
// mode: c++
// End:
