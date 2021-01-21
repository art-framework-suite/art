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
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"

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
  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~Path();
    Path(ActionTable const&,
         ActivityRegistry const&,
         PathContext const&,
         std::vector<WorkerInPath>&&,
         HLTGlobalStatus*) noexcept;
    Path(Path const&) = delete;
    Path& operator=(Path const&) = delete;

  public: // MEMBER FUNCTIONS
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
    void process_event_for_endpath(EventPrincipal&);
    void process_event(tbb::task* pathsDoneTask,
                       EventPrincipal&);

  public: // MEMBER FUNCTIONS - Tasking System
    void runWorkerTask(size_t idx,
                       size_t max_idx,
                       EventPrincipal&,
                       std::exception_ptr const*);
    void workerDoneTask(size_t idx,
                        size_t max_idx,
                        EventPrincipal&,
                        std::exception_ptr const*);

  private: // MEMBER FUNCTIONS -- Implementation details
    void process_event_idx_asynch(size_t idx, size_t max_idx, EventPrincipal&);
    void process_event_idx(size_t const idx,
                           size_t const max_idx,
                           EventPrincipal&);
    void process_event_workerFinished(size_t const idx,
                                      size_t const max_idx,
                                      EventPrincipal&,
                                      bool should_continue);
    void process_event_pathFinished(size_t const idx,
                                    EventPrincipal&,
                                    bool should_continue);

  private: // MEMBER DATA
    std::atomic<ActionTable const*> actionTable_;
    std::atomic<ActivityRegistry const*> actReg_;
    PathContext const pc_;
    int const bitpos_;
    // Note: threading: We clear their counters.
    std::atomic<std::vector<WorkerInPath>*> workers_;
    // The PathManager trigger paths info actually owns this.
    // Note: For the end path this will be the nullptr.
    std::atomic<HLTGlobalStatus*> trptr_;
    // Tasks waiting for path workers to finish.
    std::atomic<hep::concurrency::WaitingTaskList*> waitingTasks_;
    std::atomic<hlt::HLTState> state_;
    std::atomic<std::size_t> timesRun_;
    std::atomic<std::size_t> timesPassed_;
    std::atomic<std::size_t> timesFailed_;
    std::atomic<std::size_t> timesExcept_;
  };
} // namespace art

#endif /* art_Framework_Core_Path_h */

// Local Variables:
// mode: c++
// End:
