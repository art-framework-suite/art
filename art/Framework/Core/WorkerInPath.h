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
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/Transition.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "hep_concurrency/WaitingTask.h"

#include <string>

namespace art {
  using module_label_t = std::string;

  namespace detail {
    struct ModuleConfigInfo;
  }

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
    WorkerInPath(Worker*,
                 detail::FilterAction,
                 ModuleContext const&,
                 GlobalTaskGroup& group);

    // API for user
    Worker* getWorker() const;
    detail::FilterAction filterAction() const;

    // Used only by Path
    bool returnCode() const;
    std::string const& label() const;
    bool run(Transition, Principal&);
    void run(hep::concurrency::WaitingTaskPtr workerDoneTask, EventPrincipal&);
    void clearCounters();

    // Used by writeSummary
    std::size_t timesVisited() const;
    std::size_t timesPassed() const;
    std::size_t timesFailed() const;
    std::size_t timesExcept() const;

  private:
    class WorkerInPathDoneTask;

    cet::exempt_ptr<Worker> worker_;
    detail::FilterAction filterAction_;
    ModuleContext moduleContext_;
    GlobalTaskGroup* taskGroup_;

    // Per-schedule
    bool returnCode_{false};

    // Counts
    std::size_t counts_visited_{};
    std::size_t counts_passed_{};
    std::size_t counts_failed_{};
    std::size_t counts_thrown_{};
  };

} // namespace art

#endif /* art_Framework_Core_WorkerInPath_h */

// Local Variables:
// mode: c++
// End:
