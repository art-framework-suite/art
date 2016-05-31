#ifndef art_Framework_Core_WorkerInPath_h
#define art_Framework_Core_WorkerInPath_h

// ======================================================================
//
// WorkerInPath: A wrapper around a Worker, so that statistics can be
//               managed per path.  A Path holds Workers as these things.
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/MaybeRunStopwatch.h"
#include "art/Framework/Principal/Worker.h"

#include <memory>
#include <utility>

// ----------------------------------------------------------------------

namespace art {

  class WorkerInPath {
  public:
    enum FilterAction { Normal=0, Ignore, Veto };

    explicit WorkerInPath(Worker*);
    WorkerInPath(Worker*, FilterAction theAction);

    template <typename T>
    bool runWorker(typename T::MyPrincipal&,
                   CurrentProcessingContext const* cpc);

    std::pair<double,double> timeCpuReal() const
    {
      return std::pair<double,double>(stopwatch_.cpuTime(), stopwatch_.realTime());
    }

    void clearCounters()
    {
      counts_ = Counts_t{};
    }

    std::size_t timesVisited() const { return counts_.times<stats::Visited>(); }
    std::size_t timesPassed() const { return counts_.times<stats::Passed>(); }
    std::size_t timesFailed() const { return counts_.times<stats::Failed>(); }
    std::size_t timesExcept() const { return counts_.times<stats::ExceptionThrown>(); }

    FilterAction filterAction() const { return filterAction_; }
    Worker* getWorker() const { return worker_; }

    std::string const &label() const { return worker_->label(); }
    bool modifiesEvent() const { return worker_->modifiesEvent(); }

  private:
    Stopwatch::timer_type stopwatch_ {};

    using Counts_t = ExecutionCounts<stats::Visited, stats::Passed, stats::Failed, stats::ExceptionThrown>;
    Counts_t counts_ {};

    FilterAction filterAction_ {Normal};
    Worker* worker_;
  };  // WorkerInPath

  template <typename T>
  bool WorkerInPath::runWorker(typename T::MyPrincipal& ep,
                               CurrentProcessingContext const* cpc)
  {
    MaybeRunStopwatch<T::isEvent_> sentry {stopwatch_};
    counts_.increment<T::isEvent_, stats::Visited>();

    bool rc {true};
    try {
      // may want to change the return value from the worker to be the
      // Worker::FilterAction so conditions in the path will be easier
      // to identify
      rc = worker_->doWork<T>(ep, cpc);
    }
    catch(...) {
      counts_.increment<T::isEvent_, stats::ExceptionThrown>();
      throw;
    }

    // Ignore return code for non-event (e.g. run, subRun) calls
    if (T::isEvent_ && filterAction_ == Veto) rc = !rc;
    else if (!T::isEvent_ || filterAction_ == Ignore) rc = true;

    counts_.update<T::isEvent_>(rc);
    return rc;
  }  // runWorker<>()

}  // art

// ======================================================================

#endif /* art_Framework_Core_WorkerInPath_h */

// Local Variables:
// mode: c++
// End:
