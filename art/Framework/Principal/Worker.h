#ifndef art_Framework_Principal_Worker_h
#define art_Framework_Principal_Worker_h
// vim: set sw=2 expandtab :

//
// Worker: this is a basic scheduling unit - an abstract base class to
// something that is really a producer or filter.
//
//
// A worker will not actually call through to the module unless it is in
// a Ready state.  After a module is actually run, the state will not be
// Ready.  The Ready state can only be reestablished by doing a reset().
//
// Pre/post module signals are posted only in the Ready state.
//
// Execution statistics are kept here.
//
// If a module has thrown an exception during execution, that exception
// will be rethrown if the worker is entered again and the state is not
// Ready.  In other words, execution results (status) are cached and
// reused until the worker is reset().
//

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/BranchActionType.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/MaybeIncrementCounts.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <atomic>
#include <cassert>
#include <exception>
#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

namespace art {

  class ActivityRegistry;
  class FileBlock;
  class RunPrincipal;
  class SubRunPrincipal;
  class EventPrincipal;

  class Worker {

  public: // TYPES

    enum State : int {
      Ready // 0
      , Pass // 1
      , Fail // 2
      , Working // 3
      , ExceptionThrown // 4
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions

    virtual
    ~Worker() noexcept;

    Worker(ModuleDescription const&, WorkerParams const&);

    Worker(Worker const&) = delete;

    Worker(Worker&) = delete;

  public: // MEMBER FUNCTIONS -- API exposed to EventProcessor, Schedule, and EndPathExecutor

    void
    beginJob();

    void
    endJob();

    void
    respondToOpenInputFile(FileBlock const& fb);

    void
    respondToCloseInputFile(FileBlock const& fb);

    void
    respondToOpenOutputFiles(FileBlock const& fb);

    void
    respondToCloseOutputFiles(FileBlock const& fb);

    bool
    doWork(Transition, Principal&, CurrentProcessingContext*);

    void
    doWork_event(hep::concurrency::WaitingTask* workerInPathDoneTask, EventPrincipal&, int streamIndex,
                 CurrentProcessingContext*);

    // This is used to do trigger results insertion,
    // and to run workers on the end path.
    void
    doWork_event(EventPrincipal&, int streamIndex, CurrentProcessingContext*);

    ModuleDescription const&
    description() const;

    ModuleDescription const*
    descPtr() const;

    std::string const&
    label() const;

    // Unused by the framework.
    //State
    //state(int streamIndex) const;

    // Used only by WorkerInPath.
    bool
    returnCode(int streamIndex) const;

    //FIXME: Unused, remove!
    ModuleThreadingType
    moduleThreadingType() const;

    hep::concurrency::SerialTaskQueueChain*
    serialTaskQueueChain() const;

    // Used by EventProcessor
    // Used by Schedule
    // Used by EndPathExecutor
    void
    reset(int streamIndex);

    //void
    //clearCounters();

    // Used only by writeSummary
    std::size_t
    timesVisited() const;

    // Used only by writeSummary
    std::size_t
    timesRun() const;

    // Used only by writeSummary
    std::size_t
    timesPassed() const;

    // Used only by writeSummary
    std::size_t
    timesFailed() const;

    // Used only by writeSummary
    std::size_t
    timesExcept() const;

  protected: // MEMBER FUNCTIONS -- API implementation classes must provide to us

    virtual
    std::string
    workerType() const = 0;

    virtual
    hep::concurrency::SerialTaskQueueChain*
    implSerialTaskQueueChain() const = 0;

    virtual
    void
    implBeginJob() = 0;

    virtual
    void
    implEndJob() = 0;

    virtual
    bool
    implDoBegin(RunPrincipal& rp, CurrentProcessingContext* cpc) = 0;

    virtual
    bool
    implDoEnd(RunPrincipal& rp, CurrentProcessingContext* cpc) = 0;

    virtual
    bool
    implDoBegin(SubRunPrincipal& srp, CurrentProcessingContext* cpc) = 0;

    virtual
    bool
    implDoEnd(SubRunPrincipal& srp, CurrentProcessingContext* cpc) = 0;

    virtual
    bool
    implDoProcess(EventPrincipal&, int streamIndex, CurrentProcessingContext* cpc) = 0;

  private: // MEMBER FUNCTIONS -- API implementation classes must use to provide their API to us

    virtual
    void
    implRespondToOpenInputFile(FileBlock const& fb) = 0;

    virtual
    void
    implRespondToCloseInputFile(FileBlock const& fb) = 0;

    virtual
    void
    implRespondToOpenOutputFiles(FileBlock const& fb) = 0;

    virtual
    void
    implRespondToCloseOutputFiles(FileBlock const& fb) = 0;

  private: // MEMBER DATA

    ModuleDescription const
    md_;

    ActionTable const&
    actions_;

    ActivityRegistry&
    actReg_;

    ModuleThreadingType
    moduleThreadingType_{};

    std::atomic<int>
    state_{Ready};

    // if state is 'exception'
    // Note: threading: There is no accessor for this data,
    // the only way it is ever used is from the doWork*
    // functions.  Right now event processing only sets it,
    // but run and subrun processing reads it.  It is not
    // clear that event processing needs this anymore,
    // and if we go to multiple runs and subruns in flight,
    // they may not need it anymore as well.  For now, leave
    // this, is not thread safe.
    std::exception_ptr
    cached_exception_{};

    std::atomic<bool>
    workStarted_{false};

    std::atomic<bool>
    returnCode_{false};

    // Holds the waiting workerInPathDone tasks.
    // Note: For legacy, one, and global modules the workers are
    // shared.  For stream modules each stream has its own
    // private worker copies (the whole reason streams exist!).
    hep::concurrency::WaitingTaskList
    waitingTasks_{};

  protected: // MEMBER DATA -- counts

    std::atomic<std::size_t>
    counts_visited_{};

    std::atomic<std::size_t>
    counts_run_{};

    std::atomic<std::size_t>
    counts_passed_{};

    std::atomic<std::size_t>
    counts_failed_{};

    std::atomic<std::size_t>
    counts_thrown_{};

  };

} // namespace art

#endif /* art_Framework_Principal_Worker_h */

// Local Variables:
// mode: c++
// End:
