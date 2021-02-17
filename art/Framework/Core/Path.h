#ifndef art_Framework_Core_Path_h
#define art_Framework_Core_Path_h
// vim: set sw=2 expandtab :

// ====================================================================
// A object of type Path represents one path in a job configuration
// for a given schedule.  It holds the assigned bit position and the
// list of workers that are an event must pass through when this parh
// is processed.  The workers are held in WorkerInPath wrappers so
// that per-path execution statistics can be kept for each worker.
// ====================================================================

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskGroup.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace art {
  class ActivityRegistry;
  class EventPrincipal;
  class Path {
  public:
    Path(ActionTable const&,
         ActivityRegistry const&,
         PathContext const&,
         std::vector<WorkerInPath>&&,
         HLTGlobalStatus*) noexcept;

    // Disable copy operations
    Path(Path const&) = delete;
    Path& operator=(Path const&) = delete;

    ScheduleID scheduleID() const;
    int bitPosition() const;
    std::string const& name() const;
    std::vector<WorkerInPath> const& workersInPath() const;
    hlt::HLTState state() const;
    std::size_t timesRun() const;
    std::size_t timesPassed() const;
    std::size_t timesFailed() const;
    std::size_t timesExcept() const;
    // Note: threading: Clears the counters of workersInPath.
    void clearCounters();
    void process(Transition, Principal&);
    void process(task_ptr_t pathsDoneTask, EventPrincipal&);

  private:
    class RunWorkerTask;
    class WorkerDoneTask;

    void process_event_idx_asynch(size_t idx,
                                  size_t max_idx,
                                  EventPrincipal&,
                                  task_ptr_t pathsDone);
    void process_event_idx(size_t const idx,
                           size_t const max_idx,
                           EventPrincipal&,
                           task_ptr_t pathsDone);
    void process_event_workerFinished(size_t const idx,
                                      size_t const max_idx,
                                      EventPrincipal& ep,
                                      bool should_continue,
                                      task_ptr_t pathsDone);
    void process_event_pathFinished(size_t const idx,
                                    bool should_continue,
                                    task_ptr_t pathsDone);

    ActionTable const& actionTable_;
    ActivityRegistry const& actReg_;
    PathContext const pc_;
    int const bitpos_;
    // Note: threading: We clear their counters.
    std::vector<WorkerInPath> workers_;
    // The PathManager trigger paths info actually owns this.
    // Note: For the end path this will be the nullptr.
    std::atomic<HLTGlobalStatus*> trptr_;
    std::atomic<hlt::HLTState> state_{hlt::Ready};
    std::atomic<std::size_t> timesRun_{};
    std::atomic<std::size_t> timesPassed_{};
    std::atomic<std::size_t> timesFailed_{};
    std::atomic<std::size_t> timesExcept_{};
  };
} // namespace art

#endif /* art_Framework_Core_Path_h */

// Local Variables:
// mode: c++
// End:
