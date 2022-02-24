#ifndef art_Framework_Core_TriggerPathsExecutor_h
#define art_Framework_Core_TriggerPathsExecutor_h
// vim: set sw=2 expandtab :

// ======================================================================
// TriggerPathsExecutor
//
// A trigger-paths executor is a sequence of trigger paths. After
// construction, events can be fed to the object and passed through
// all the producer and filter modules. All accounting about
// processing of events by modules and paths is contained here or in
// an object held by containment.
//
// The trigger results producer is generated and managed here. This
// class also manages calls to endjob and beginjob.
//
// A TriggerResults object will always be inserted into the event for
// any schedule. The producer of the TriggerResults EDProduct is
// always the last module in the trigger path. The
// TriggerResultInserter is given a fixed label of "TriggerResults".
//
// Processing of an event happens by pushing the event through the
// Paths. The scheduler performs the reset() on each of the workers
// independent of the Path objects.
// ======================================================================

#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"

#include <memory>

namespace art {
  class ActivityRegistry;
  class GlobalTaskGroup;
  namespace detail {
    class SharedResources;
  }

  class TriggerPathsExecutor {
  public:
    TriggerPathsExecutor(ScheduleID,
                         PathManager&,
                         ActionTable const&,
                         ActivityRegistry const& activityRegistry,
                         GlobalTaskGroup& group);

    // Disable copy/move operations
    TriggerPathsExecutor(TriggerPathsExecutor const&) = delete;
    TriggerPathsExecutor(TriggerPathsExecutor&&) = delete;
    TriggerPathsExecutor& operator=(TriggerPathsExecutor const&) = delete;
    TriggerPathsExecutor& operator=(TriggerPathsExecutor&&) = delete;

    // API presented to EventProcessor
    void process(Transition, Principal&);
    void process_event(hep::concurrency::WaitingTaskPtr endPathTask,
                       EventPrincipal&);
    void beginJob(detail::SharedResources const& resources);
    void endJob();
    void respondToOpenInputFile(FileBlock const&);
    void respondToCloseInputFile(FileBlock const&);
    void respondToOpenOutputFiles(FileBlock const&);
    void respondToCloseOutputFiles(FileBlock const&);

    void process_event_paths_done(EventPrincipal&);

  private:
    class PathsDoneTask;

    bool skipNonReplicated_(Worker const&);

    // const after ctor.
    ScheduleContext const sc_;
    ActionTable const& actionTable_;
    ActivityRegistry const& actReg_;
    PathsInfo& triggerPathsInfo_;
    std::unique_ptr<Worker> results_inserter_;
    GlobalTaskGroup& taskGroup_;
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_TriggerPathsExecutor_h */
