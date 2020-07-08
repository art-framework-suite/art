#ifndef art_Framework_Principal_Worker_h
#define art_Framework_Principal_Worker_h
// vim: set sw=2 expandtab :

// ======================================================================
// Worker: this is a basic scheduling unit - an abstract base class to
// something that is really a producer or filter.
//
// A worker will not actually call through to the module unless it is
// in a Ready state.  After a module is actually run, the state will
// not be Ready.  The Ready state can only be reestablished by doing a
// reset().
//
// Pre/post module signals are posted only in the Ready state.
//
// Execution statistics are kept here.
//
// If a module has thrown an exception during execution, that
// exception will be rethrown if the worker is entered again and the
// state is not Ready.  In other words, execution results (status) are
// cached and reused until the worker is reset().
// ======================================================================

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/BranchActionType.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/MaybeIncrementCounts.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <atomic>
#include <cassert>
#include <exception>
#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

namespace tbb {
  class task;
}

namespace hep::concurrency {
  class SerialTaskQueueChain;
  class WaitingTaskList;
  using WaitingTask = tbb::task;
}

namespace art {

  class ActivityRegistry;
  class ModuleContext;
  class FileBlock;
  class RunPrincipal;
  class SubRunPrincipal;
  class EventPrincipal;

  class Worker {

    friend class RunWorkerFunctor;

  public: // TYPES
    enum State : int {
      Ready = 0,
      Pass = 1,
      Fail = 2,
      Working = 3,
      ExceptionThrown = 4
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~Worker() noexcept;
    Worker(ModuleDescription const&, WorkerParams const&);
    Worker(Worker const&) = delete;
    Worker(Worker&) = delete;

  public: // MEMBER FUNCTIONS -- API exposed to EventProcessor, Schedule,
          // and EndPathExecutor
    void beginJob();
    void endJob();
    void respondToOpenInputFile(FileBlock const& fb);
    void respondToCloseInputFile(FileBlock const& fb);
    void respondToOpenOutputFiles(FileBlock const& fb);
    void respondToCloseOutputFiles(FileBlock const& fb);
    bool doWork(Transition, Principal&, ModuleContext const&);

    void doWork_event(hep::concurrency::WaitingTask* workerInPathDoneTask,
                      EventPrincipal&,
                      ModuleContext const&);

    // This is used to do trigger results insertion,
    // and to run workers on the end path.
    void doWork_event(EventPrincipal&, ModuleContext const&);

    ScheduleID
    scheduleID() const
    {
      return scheduleID_;
    }
    ModuleDescription const& description() const;
    ModuleDescription const* descPtr() const;
    std::string const& label() const;

    // Used only by WorkerInPath.
    bool returnCode() const;

    hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;

    // Used by EventProcessor
    // Used by Schedule
    // Used by EndPathExecutor
    void reset();

    // Used only by writeSummary
    std::size_t timesVisited() const;
    std::size_t timesRun() const;
    std::size_t timesPassed() const;
    std::size_t timesFailed() const;
    std::size_t timesExcept() const;

  public: // Tasking Structure
    void runWorker(EventPrincipal&, ModuleContext const&);

  protected: // MEMBER FUNCTIONS -- API implementation classes must provide
    virtual std::string workerType() const = 0;
    virtual hep::concurrency::SerialTaskQueueChain* implSerialTaskQueueChain()
      const = 0;
    virtual void implBeginJob() = 0;
    virtual void implEndJob() = 0;
    virtual bool implDoBegin(RunPrincipal& rp, ModuleContext const& mc) = 0;
    virtual bool implDoEnd(RunPrincipal& rp, ModuleContext const& mc) = 0;
    virtual bool implDoBegin(SubRunPrincipal& srp, ModuleContext const& mc) = 0;
    virtual bool implDoEnd(SubRunPrincipal& srp, ModuleContext const& mc) = 0;
    virtual bool implDoProcess(EventPrincipal&, ModuleContext const&) = 0;

  private: // MEMBER FUNCTIONS -- API implementation classes must use
           // to provide their API to us
    virtual void implRespondToOpenInputFile(FileBlock const& fb) = 0;
    virtual void implRespondToCloseInputFile(FileBlock const& fb) = 0;
    virtual void implRespondToOpenOutputFiles(FileBlock const& fb) = 0;
    virtual void implRespondToCloseOutputFiles(FileBlock const& fb) = 0;

  private: // MEMBER DATA
    ScheduleID const scheduleID_;
    std::atomic<ModuleDescription const*> md_;
    std::atomic<ActionTable const*> actions_;
    std::atomic<ActivityRegistry const*> actReg_;
    std::atomic<int> state_;

    // if state is 'exception'
    // Note: threading: There is no accessor for this data,
    // the only way it is ever used is from the doWork*
    // functions.  Right now event processing only sets it,
    // but run and subrun processing reads it.  It is not
    // clear that event processing needs this anymore,
    // and if we go to multiple runs and subruns in flight,
    // they may not need it anymore as well.  For now, leave
    // this, is not thread safe.
    std::atomic<std::exception_ptr*> cached_exception_;

    std::atomic<bool> workStarted_;
    std::atomic<bool> returnCode_;

    // Holds the waiting workerInPathDone tasks.  Note: For shared
    // modules the workers are shared.  For replicated modules each
    // schedule has its own private worker copies (the whole reason
    // schedules exist!).
    std::atomic<hep::concurrency::WaitingTaskList*> waitingTasks_;

  protected: // MEMBER DATA -- counts
    std::atomic<std::size_t> counts_visited_;
    std::atomic<std::size_t> counts_run_;
    std::atomic<std::size_t> counts_passed_;
    std::atomic<std::size_t> counts_failed_;
    std::atomic<std::size_t> counts_thrown_;
  };

} // namespace art

#endif /* art_Framework_Principal_Worker_h */

// Local Variables:
// mode: c++
// End:
