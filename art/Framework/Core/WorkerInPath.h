#ifndef art_Framework_Core_WorkerInPath_h
#define art_Framework_Core_WorkerInPath_h
// vim: set sw=2 expandtab :

// ====================================================================
// The WorkerInPath is a wrapper around a Worker, so that statistics
// can be managed per path.  A Path holds Workers as these things.
//
// For a given module label, there will be n*m WorkerInPath objects,
// where n is the number of configured schedules, and m is the number
// paths in which the module label appears.
// ====================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace art {
  using module_label_t = std::string;
  class PathContext;
  class WorkerInPath {
  public: // TYPES
    enum FilterAction { Normal = 0, Ignore = 1, Veto = 2 };
    struct ConfigInfo {
      ConfigInfo(module_label_t const& lbl, FilterAction const action)
        : label{lbl}, filterAction{action}
      {}
      module_label_t label;
      FilterAction filterAction;
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~WorkerInPath() noexcept;
    explicit WorkerInPath(Worker*) noexcept;
    WorkerInPath(Worker*, FilterAction) noexcept;
    WorkerInPath(WorkerInPath const&) = delete;
    WorkerInPath(WorkerInPath&&) noexcept;
    WorkerInPath& operator=(WorkerInPath const&) = delete;
    WorkerInPath& operator=(WorkerInPath&&) noexcept;

  public: // MEMBER FUNCTIONS -- API for user
    Worker* getWorker() const;
    FilterAction filterAction() const;
    // Used only by Path
    bool returnCode(ScheduleID const) const;
    std::string const& label() const;
    bool runWorker(Transition, Principal&, PathContext const&);
    void runWorker_event_for_endpath(EventPrincipal&,
                                     ScheduleID,
                                     PathContext const&);
    void runWorker_event(hep::concurrency::WaitingTask* workerDoneTask,
                         EventPrincipal&,
                         ScheduleID,
                         PathContext const&);
    // Used only by Path
    void clearCounters();
    // Used by writeSummary
    std::size_t timesVisited() const;
    // Used by writeSummary
    std::size_t timesPassed() const;
    // Used by writeSummary
    std::size_t timesFailed() const;
    // Used by writeSummary
    std::size_t timesExcept() const;

  public: // MEMBER FUNCTIONS -- Task Structure
    void workerInPathDoneTask(ScheduleID const, std::exception_ptr const*);

  private: // MEMBER DATA
    std::atomic<Worker*> worker_;
    std::atomic<FilterAction> filterAction_;

  private: // MEMBER DATA -- Per-schedule
    std::atomic<bool> returnCode_;
    std::atomic<hep::concurrency::WaitingTaskList*> waitingTasks_;

  private: // MEMBER DATA -- Counts
    std::atomic<std::size_t> counts_visited_;
    std::atomic<std::size_t> counts_passed_;
    std::atomic<std::size_t> counts_failed_;
    std::atomic<std::size_t> counts_thrown_;
  };

} // namespace art

#endif /* art_Framework_Core_WorkerInPath_h */

// Local Variables:
// mode: c++
// End:
