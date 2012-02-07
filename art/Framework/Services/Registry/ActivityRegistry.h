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

#include "boost/noncopyable.hpp"
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

  class EventProcessor;
  class WorkerRegistry;
  class Schedule;
  enum class BranchActionType; // C++2011 Forward declared enum.
  template <typename, BranchActionType> class OccurrenceTraits;

}  // art

////////////////////////////////////////////////////////////////////////
// Helper macros (do not call yourself).
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
#define AR_DECL_SIGNAL(returnType, nArgs, stateTag, cMethod)  \
  private: \
  stateTag s##stateTag##_; \
  public: \
  returnType watch##stateTag(stateTag::slot_type const & iSlot) { \
    s##stateTag##_.cMethod(iSlot); \
  } \
  AR_DECL_STATE_##nArgs##_ARG_FUNC(stateTag)
#define AR_DECL_FIFO_SIGNAL(returnType, nArgs, stateTag) \
  AR_DECL_SIGNAL(returnType, nArgs, stateTag, connect)
#define AR_DECL_LIFO_SIGNAL(returnType, nArgs, stateTag) \
  AR_DECL_SIGNAL(returnType, nArgs, stateTag, slots().push_front)
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Top level macros (call in definition of Activity registry). Add your
// own as appropriate (and undef them at the end).
#define AR_DECL_VOID_0ARG_SIGNAL(stackType, stateTag)  \
  typedef sigc::signal<void> stateTag; \
  AR_DECL_##stackType##_SIGNAL(void, 0, stateTag)
#define AR_DECL_VOID_1ARG_SIGNAL(stackType, arg, stateTag) \
  typedef sigc::signal<void, arg> stateTag; \
  AR_DECL_##stackType##_SIGNAL(void, 1, stateTag)
#define AR_DECL_VOID_2ARG_SIGNAL(stackType, arg1, arg2, stateTag) \
  typedef sigc::signal<void, arg1, arg2> stateTag;                     \
  AR_DECL_##stackType##_SIGNAL(void, 2, stateTag)
////////////////////////////////////////////////////////////////////////

class art::ActivityRegistry : private boost::noncopyable {
public:

  friend class EventProcessor;
  friend class Worker;
  friend class WorkerRegistry;
  template <typename, BranchActionType> friend class OccurrenceTraits;

  // ---------- signals ------------------------------------
  // Signal is emitted after all modules have had their beginJob called
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PostBeginJob)

  // Signal is emitted after all modules have had their endJob called
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostEndJob)

  // Signal is emitted if event processing or end-of-job
  // processing fails with an uncaught exception.
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, JobFailure)

  // Signal is emitted before the source starts creating an Event
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreSource)

  // Signal is emitted after the source starts creating an Event
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostSource)

  // Signal is emitted before the source starts creating a SubRun
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreSourceSubRun)

  // Signal is emitted after the source starts creating a SubRun
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostSourceSubRun)

  // Signal is emitted before the source starts creating a Run
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreSourceRun)

  // Signal is emitted after the source starts creating a Run
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostSourceRun)

  // Signal is emitted before the source opens a file
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreOpenFile)

  // Signal is emitted after the source opens a file
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostOpenFile)

  // Signal is emitted before the Closesource closes a file
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreCloseFile)

  // Signal is emitted after the source opens a file
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostCloseFile)

  // Signal is emitted after the Event has been created by the
  // InputSource but before any modules have seen the Event
  AR_DECL_VOID_0ARG_SIGNAL(FIFO, PreProcessEvent)

  // Signal is emitted after all modules have finished processing the
  // Event
  AR_DECL_VOID_0ARG_SIGNAL(LIFO, PostProcessEvent)

  // Signal is emitted after the Run has been created by the InputSource
  // but before any modules have seen the Run
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, Run const &, PreBeginRun)

  // Signal is emitted after all modules have finished processing the
  // beginRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, Run const &, PostBeginRun)

  // Signal is emitted before the endRun is processed
  AR_DECL_VOID_2ARG_SIGNAL(FIFO, RunID const &, Timestamp const &, PreEndRun)

  // Signal is emitted after all modules have finished processing the
  // Run
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, Run const &, PostEndRun)

  // Signal is emitted after the SubRun has been created by the
  // InputSource but before any modules have seen the SubRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, SubRun const &, PreBeginSubRun)

  // Signal is emitted after all modules have finished processing the
  // beginSubRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, SubRun const &, PostBeginSubRun)

  // Signal is emitted before the endSubRun is processed
  AR_DECL_VOID_2ARG_SIGNAL(FIFO, SubRunID const &, Timestamp const &, PreEndSubRun)

  // Signal is emitted after all modules have finished processing the
  // SubRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, SubRun const &, PostEndSubRun)

  // Signal is emitted before starting to process a Path for an event
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, std::string const &, PreProcessPath)

  // Signal is emitted after all modules have finished for the Path for
  // an event
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, std::string const &, HLTPathStatus const &, PostProcessPath)

  // Signal is emitted before starting to process a Path for beginRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, std::string const &, PrePathBeginRun)

  // Signal is emitted after all modules have finished for the Path for beginRun
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, std::string const &, HLTPathStatus const &, PostPathBeginRun)

  // Signal is emitted before starting to process a Path for endRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, std::string const &, PrePathEndRun)

  // Signal is emitted after all modules have finished for the Path for endRun
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, std::string const &, HLTPathStatus const &, PostPathEndRun)

  // Signal is emitted before starting to process a Path for beginSubRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, std::string const &, PrePathBeginSubRun)

  // Signal is emitted after all modules have finished for the Path for beginSubRun
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, std::string const &, HLTPathStatus const &, PostPathBeginSubRun)

  // Signal is emitted before starting to process a Path for endRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, std::string const &, PrePathEndSubRun)

  // Signal is emitted after all modules have finished for the Path for endRun
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, std::string const &, HLTPathStatus const &, PostPathEndSubRun)

  // Signal is emitted before the module is constructed
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleConstruction)

  // Signal is emitted after the module was construction
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleConstruction)

  // JBK added
  // Signal is emitted after beginJob
  AR_DECL_VOID_2ARG_SIGNAL(LIFO, InputSource *, std::vector<Worker *> const &, PostBeginJobWorkers)
  // end JBK added

  // Signal is emitted before the module does beginJob
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleBeginJob)

  // Signal is emitted after the module had done beginJob
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleBeginJob)

  // Signal is emitted before the module does endJob
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleEndJob)

  // Signal is emitted after the module had done endJob
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleEndJob)

  // Signal is emitted before the module starts processing the Event
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModule)

  // Signal is emitted after the module finished processing the Event
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModule)

  // Signal is emitted before the module starts processing beginRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleBeginRun)

  // Signal is emitted after the module finished processing beginRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleBeginRun)

  // Signal is emitted before the module starts processing endRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleEndRun)

  // Signal is emitted after the module finished processing endRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleEndRun)

  // Signal is emitted before the module starts processing beginSubRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleBeginSubRun)

  // Signal is emitted after the module finished processing beginSubRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleBeginSubRun)

  // Signal is emitted before the module starts processing endSubRun
  AR_DECL_VOID_1ARG_SIGNAL(FIFO, ModuleDescription const &, PreModuleEndSubRun)

  // Signal is emitted after the module finished processing endSubRun
  AR_DECL_VOID_1ARG_SIGNAL(LIFO, ModuleDescription const &, PostModuleEndSubRun)

  // ---------- member functions ---------------------------

  // Forwards our signals to slots connected to iOther
  void connect(ActivityRegistry & iOther);

  // Copy the slots from iOther and connect them directly to our own
  // this allows us to 'forward' signals more efficiently, BUT if iOther
  // gains new slots after this call, we will not see them This is also
  // careful to keep the order of the slots proper for services.
  void copySlotsFrom(ActivityRegistry & iOther);

};  // ActivityRegistry

#undef AR_DECL_STATE_0_ARG_FUNC
#undef AR_DECL_STATE_1_ARG_FUNC
#undef AR_DECL_STATE_2_ARG_FUNC
#undef AR_DECL_SIGNAL
#undef AR_DECL_LIFO_SIGNAL
#undef AR_DECL_FIFO_SIGNAL
#undef AR_DECL_VOID_0ARG_SIGNAL
#undef AR_DECL_VOID_1ARG_SIGNAL
#undef AR_DECL_VOID_2ARG_SIGNAL
#endif /* art_Framework_Services_Registry_ActivityRegistry_h */

// Local Variables:
// mode: c++
// End:
