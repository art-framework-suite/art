#ifndef art_Framework_Core_WorkerInPath_h
#define art_Framework_Core_WorkerInPath_h
// vim: set sw=2 expandtab :

//
// A wrapper around a Worker, so that statistics can be
// managed per path.  A Path holds Workers as these things.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

namespace art {

  using module_label_t = std::string;

  class WorkerInPath {
  public: // TYPES
    enum FilterAction {
      Normal = 0,
      Ignore // 1
      ,
      Veto // 2
    };

    struct ConfigInfo {
      ConfigInfo(module_label_t const& lbl, FilterAction const action)
        : label{lbl}, filterAction{action}
      {}
      module_label_t label; // Used for looking up ModuleConfigInfo
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
    bool returnCode(ScheduleID scheduleID) const;
    std::string const& label() const;
    bool runWorker(Transition, Principal&, CurrentProcessingContext*);

    void runWorker_event_for_endpath(EventPrincipal&,
                                     ScheduleID scheduleID,
                                     CurrentProcessingContext*);

    void runWorker_event(hep::concurrency::WaitingTask* workerDoneTask,
                         EventPrincipal&,
                         ScheduleID scheduleID,
                         CurrentProcessingContext*);

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

  private: // MEMBER DATA
    Worker* worker_{nullptr};

    FilterAction filterAction_{Normal};

  private: // MEMBER DATA -- Per-schedule
    bool returnCode_{false};

    hep::concurrency::WaitingTaskList waitingTasks_{};

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
