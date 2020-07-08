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
#include "art/Framework/Core/detail/ModuleKeyAndType.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/Transition.h"
#include "cetlib/exempt_ptr.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace art {
  using module_label_t = std::string;

  namespace detail {
    struct ModuleConfigInfo;
  }

  class PathContext;

  class WorkerInPath {
  public:
    struct ConfigInfo {
      ConfigInfo(cet::exempt_ptr<detail::ModuleConfigInfo const> const info,
                 detail::FilterAction const action)
        : moduleConfigInfo{info}, filterAction{action}
      {}
      cet::exempt_ptr<detail::ModuleConfigInfo const> moduleConfigInfo;
      detail::FilterAction filterAction;
    };

    // Special Member Functions
    ~WorkerInPath() noexcept;
    WorkerInPath(Worker*, detail::FilterAction, ModuleContext const&);
    WorkerInPath(WorkerInPath const&) = delete;
    WorkerInPath(WorkerInPath&&);
    WorkerInPath& operator=(WorkerInPath const&) = delete;
    WorkerInPath& operator=(WorkerInPath&&);

    // API for user
    Worker* getWorker() const;
    detail::FilterAction filterAction() const;

    // Used only by Path
    bool returnCode() const;
    std::string const& label() const;
    bool runWorker(Transition, Principal&);
    void runWorker_event_for_endpath(EventPrincipal&);
    void runWorker_event(hep::concurrency::WaitingTask* workerDoneTask,
                         EventPrincipal&);
    // Used only by Path
    void clearCounters();

    // Used by writeSummary
    std::size_t timesVisited() const;
    std::size_t timesPassed() const;
    std::size_t timesFailed() const;
    std::size_t timesExcept() const;

    // Task Structure
    void workerInPathDoneTask(ScheduleID const, std::exception_ptr const*);

  private:
    std::atomic<Worker*> worker_;
    std::atomic<detail::FilterAction> filterAction_;
    ModuleContext moduleContext_;

    // Per-schedule
    std::atomic<bool> returnCode_;
    std::atomic<hep::concurrency::WaitingTaskList*> waitingTasks_;

    // Counts
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
