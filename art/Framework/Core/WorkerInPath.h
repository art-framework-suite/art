#ifndef art_Framework_Core_WorkerInPath_h
#define art_Framework_Core_WorkerInPath_h

// ======================================================================
//
// WorkerInPath: A wrapper around a Worker, so that statistics can be
//               managed per path.  A Path holds Workers as these things.
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "cpp0x/memory"
#include "cpp0x/utility"

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

    std::pair<double,double> timeCpuReal() const {
      return std::pair<double,double>(stopwatch_->cpuTime(),stopwatch_->realTime());
    }

    void clearCounters() {
      timesVisited_ = timesPassed_ = timesFailed_ = timesExcept_ = 0;
    }

    int timesVisited() const { return timesVisited_; }
    int timesPassed() const { return timesPassed_; }
    int timesFailed() const { return timesFailed_; }
    int timesExcept() const { return timesExcept_; }

    FilterAction filterAction() const { return filterAction_; }
    Worker* getWorker() const { return worker_; }

    std::string const &label() const { return worker_->label(); }
    bool modifiesEvent() const { return worker_->modifiesEvent(); }

  private:
    RunStopwatch::StopwatchPointer stopwatch_;

    int timesVisited_;
    int timesPassed_;
    int timesFailed_;
    int timesExcept_;

    FilterAction filterAction_;
    Worker* worker_;
  };  // WorkerInPath

  template <typename T>
  bool WorkerInPath::runWorker(typename T::MyPrincipal & ep,
                               CurrentProcessingContext const* cpc) {

    // A RunStopwatch, but only if we are processing an event.
    std::auto_ptr<RunStopwatch> stopwatch(T::isEvent_ ? new RunStopwatch(stopwatch_) : 0);

    if (T::isEvent_) {
      ++timesVisited_;
    }
    bool rc = true;

    try {
        // may want to change the return value from the worker to be
        // the Worker::FilterAction so conditions in the path will be easier to
        // identify
        rc = worker_->doWork<T>(ep, cpc);

        // Ignore return code for non-event (e.g. run, subRun) calls
        if (!T::isEvent_) rc = true;
        else if (filterAction_ == Veto) rc = !rc;
        else if (filterAction_ == Ignore) rc = true;

        if (T::isEvent_) {
          if(rc) ++timesPassed_; else ++timesFailed_;
        }
    }
    catch(...) {
        if (T::isEvent_) ++timesExcept_;
        throw;
    }

    return rc;
  }  // runWorker<>()

}  // art

// ======================================================================

#endif /* art_Framework_Core_WorkerInPath_h */

// Local Variables:
// mode: c++
// End:
