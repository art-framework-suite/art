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

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  class TriggerPathsExecutor {
  public:
    TriggerPathsExecutor(ScheduleID,
                         PathManager&,
                         ActionTable const&,
                         std::unique_ptr<Worker> triggerResultsInserter);

    // Disable copy/move operations
    TriggerPathsExecutor(TriggerPathsExecutor const&) = delete;
    TriggerPathsExecutor(TriggerPathsExecutor&&) = delete;
    TriggerPathsExecutor& operator=(TriggerPathsExecutor const&) = delete;
    TriggerPathsExecutor& operator=(TriggerPathsExecutor&&) = delete;

    // API presented to EventProcessor
    void process(Transition, Principal&);
    void process_event(task_ptr_t endPathTask, EventPrincipal&);
    void beginJob();
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
    PathsInfo& triggerPathsInfo_;
    std::unique_ptr<Worker> results_inserter_;
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_TriggerPathsExecutor_h */
