#ifndef art_Framework_Principal_Worker_h
#define art_Framework_Principal_Worker_h

// ======================================================================
/*

Worker: this is a basic scheduling unit - an abstract base class to
something that is really a producer or filter.


A worker will not actually call through to the module unless it is in
a Ready state.  After a module is actually run, the state will not be
Ready.  The Ready state can only be reestablished by doing a reset().

Pre/post module signals are posted only in the Ready state.

Execution statistics are kept here.

If a module has thrown an exception during execution, that exception
will be rethrown if the worker is entered again and the state is not
Ready.  In other words, execution results (status) are cached and
reused until the worker is reset().

*/
// ======================================================================

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/BranchActionType.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/MaybeIncrementCounts.h"
#include "art/Framework/Principal/MaybeRunStopwatch.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iosfwd>
#include <memory>
#include <utility>

// ----------------------------------------------------------------------

namespace art {
  class ActivityRegistry;
  class EventPrincipal;
  class FileBlock;
  class RunPrincipal;
  class SubRunPrincipal;
}

class art::Worker {
public:
  enum State { Ready, Pass, Fail, Working, Exception };

  Worker(ModuleDescription const& iMD, WorkerParams const& iWP);
  virtual ~Worker() noexcept = default;

  template <typename T>
  bool doWork(typename T::MyPrincipal&,
              CurrentProcessingContext const* cpc);
  void beginJob() ;
  void endJob();
  void respondToOpenInputFile(FileBlock const& fb);
  void respondToCloseInputFile(FileBlock const& fb);
  void respondToOpenOutputFiles(FileBlock const& fb);
  void respondToCloseOutputFiles(FileBlock const& fb);

  void reset() { state_ = Ready; }

  ModuleDescription const& description() const {return md_;}
  ModuleDescription const* descPtr() const {return &md_; }
  ///The signals are required to live longer than the last call to 'doWork'
  /// this was done to improve performance based on profiling
  void setActivityRegistry(cet::exempt_ptr<ActivityRegistry> areg);

  std::pair<double,double> timeCpuReal() const
  {
    return std::pair<double,double>(timer_.cpuTime(),timer_.realTime());
  }

  void clearCounters()
  {
    counts_ = CountingStatistics{};
  }

  std::size_t timesRun() const { return counts_.times<stats::Run>(); }
  std::size_t timesVisited() const { return counts_.times<stats::Visited>(); }
  std::size_t timesPassed() const { return counts_.times<stats::Passed>(); }
  std::size_t timesFailed() const { return counts_.times<stats::Failed>(); }
  std::size_t timesExcept() const { return counts_.times<stats::ExceptionThrown>(); }
  State state() const { return state_; }

  virtual bool modifiesEvent() const = 0;

  std::string const& label() const { return md_.moduleLabel(); }

protected:
  virtual std::string workerType() const = 0;
  virtual bool implDoProcess(EventPrincipal&,
                             CurrentProcessingContext const* cpc,
                             CountingStatistics&) = 0;
  virtual bool implDoBegin(RunPrincipal& rp,
                           CurrentProcessingContext const* cpc) = 0;
  virtual bool implDoEnd(RunPrincipal& rp,
                         CurrentProcessingContext const* cpc) = 0;
  virtual bool implDoBegin(SubRunPrincipal& srp,
                           CurrentProcessingContext const* cpc) = 0;
  virtual bool implDoEnd(SubRunPrincipal& srp,
                         CurrentProcessingContext const* cpc) = 0;
  virtual void implBeginJob() = 0;
  virtual void implEndJob() = 0;

private:

  template <BranchActionType>
  struct ImplDoWork;

  virtual void implRespondToOpenInputFile(FileBlock const& fb) = 0;
  virtual void implRespondToCloseInputFile(FileBlock const& fb) = 0;
  virtual void implRespondToOpenOutputFiles(FileBlock const& fb) = 0;
  virtual void implRespondToCloseOutputFiles(FileBlock const& fb) = 0;

  cet::cpu_timer timer_ {};

  CountingStatistics counts_ {};
  State state_ {Ready};

  ModuleDescription md_;
  ActionTable const& actions_;
  std::shared_ptr<art::Exception> cached_exception_ {nullptr}; // if state is 'exception'
  cet::exempt_ptr<ActivityRegistry> actReg_ {nullptr};
};

namespace art {
  namespace detail {
    template <typename T> class ModuleSignalSentry;
    template <typename T>
    cet::exception &
    exceptionContext(ModuleDescription const &md,
                     T const &ip,
                     cet::exception &ex);
  }

  template <>
  struct Worker::ImplDoWork<BranchActionBegin> {
    template <typename PRINCIPAL>
    static bool invoke(Worker* const w,
                       PRINCIPAL& p,
                       CurrentProcessingContext const* cpc)
    {
      return w->implDoBegin(p, cpc);
    }
  };

  template <>
  struct Worker::ImplDoWork<BranchActionEnd> {
    template <typename PRINCIPAL>
    static bool invoke(Worker* const w,
                       PRINCIPAL& p,
                       CurrentProcessingContext const* cpc)
    {
      return w->implDoEnd(p, cpc);
    }
  };

  template <>
  struct Worker::ImplDoWork<BranchActionProcess> {
    template <typename PRINCIPAL>
    static bool invoke(Worker* const w,
                       PRINCIPAL& p,
                       CurrentProcessingContext const* cpc)
    {
      return w->implDoProcess(p, cpc, w->counts_);
    }
  };

}

template <typename T>
class art::detail::ModuleSignalSentry {
public:
  ModuleSignalSentry(ActivityRegistry *a, ModuleDescription& md) : a_{a}, md_{&md} {
    if(a_) T::preModuleSignal(a_, md_);
  }
  ~ModuleSignalSentry() {
    if(a_) T::postModuleSignal(a_, md_);
  }
private:
  ActivityRegistry* a_;
  ModuleDescription* md_;
};

template <typename T>
cet::exception&
art::detail::
exceptionContext(ModuleDescription const& iMD,
                 T const& ip,
                 cet::exception& iEx)
{
  iEx << iMD.moduleName() << "/" << iMD.moduleLabel()
      << " " << ip.id() << "\n";
  return iEx;
}

template <typename T>
bool art::Worker::doWork(typename T::MyPrincipal& p,
                         CurrentProcessingContext const* cpc)
{
  MaybeIncrementCounts<T::level, decltype(counts_)> counts {counts_};
  counts.template increment<stats::Visited>();

  switch(state_) {
  case Ready: break;
  case Pass: return true;
  case Fail: return false;
  case Exception:
    {
      // Rethrow the cached exception again. It seems impossible to
      // get here a second time unless a cet::exception has been
      // thrown previously.
      mf::LogWarning("repeat")
        << "A module has been invoked a second time even though"
        " it caught an exception during the previous invocation."
        "\nThis may be an indication of a configuration problem.\n";
      throw *cached_exception_;
    }
  case Working: break; // See below.
  }

  bool rc {false};
  try {
    if (state_ == Working) {
      // Not part of the switch statement above because we want the
      // exception to be caught by our handling mechanism.
      throw art::Exception(errors::ScheduleExecutionFailure)
        << "A Module has been invoked while it is still being executed.\n"
        << "Product dependencies have invoked a module execution cycle.\n";
    }

    detail::ModuleSignalSentry<T> cpp {actReg_.get(), md_};
    state_ = Working;
    rc = ImplDoWork<T::processing_action>::invoke(this, p, cpc);
    state_ = Pass;

    if (T::level == Level::Event && !rc)
      state_ = Fail;
  }

  catch(cet::exception& e) {

    // NOTE: the warning printed as a result of ignoring or failing a
    // module will only be printed during the full true processing
    // pass of this module.

    // Get the action corresponding to this exception.  However, if
    // processing something other than an event (e.g. run, subRun)
    // always rethrow.
    actions::ActionCodes action {T::level == Level::Event ? actions_.find(e.root_cause()) : actions::Rethrow};

    // If we are processing an endPath, treat SkipEvent or FailPath as
    // FailModule, so any subsequent OutputModules are still run.
    if (cpc && cpc->isEndPath()) {
      if (action == actions::SkipEvent || action == actions::FailPath) action = actions::FailModule;
    }
    switch(action) {
    case actions::IgnoreCompletely: {
      rc=true;
      counts.template increment<stats::Passed>();
      state_ = Pass;
      mf::LogWarning("IgnoreCompletely")
        << "Module ignored an exception\n"
        << e.what() << "\n";
      break;
    }

    case actions::FailModule: {
      rc=true;
      mf::LogWarning("FailModule")
        << "Module failed due to an exception\n"
        << e.what() << "\n";
      counts.template increment<stats::Failed>();
      state_ = Fail;
      break;
    }

    default: {

      // We should not need to include the event/run/module names.
      // The exception because the error logger will pick this up
      // automatically.  I'm leaving it in until this is verified.

      // here we simply add a small amount of data to the exception to
      // add some context, we could have rethrown it as something else
      // and embedded with this exception as an argument to the
      // constructor.
      counts.template increment<stats::ExceptionThrown>();
      state_ = Exception;
      e << "cet::exception going through module ";
      //      detail::exceptionContext(md_, p, e);
      if (auto edmEx = dynamic_cast<art::Exception*>(&e)) {
        cached_exception_ = std::make_shared<art::Exception>(*edmEx);
      } else {
        cached_exception_ = std::make_shared<art::Exception>(errors::OtherArt, std::string(), e);
      }
      throw;
    }
    }
  }

  catch(std::bad_alloc const& bda) {
    counts.template increment<stats::ExceptionThrown>();
    state_ = Exception;
    cached_exception_ = std::make_shared<art::Exception>(errors::BadAlloc);
    *cached_exception_
      << "A std::bad_alloc exception occurred during a call to the module ";
    detail::exceptionContext(md_, p, *cached_exception_)
      << "The job has probably exhausted the virtual memory available to the process.\n";
    throw *cached_exception_;
  }
  catch(std::exception const& e) {
    counts.template increment<stats::ExceptionThrown>();
    state_ = Exception;
    cached_exception_ = std::make_shared<art::Exception>(errors::StdException);
    *cached_exception_
      << "A std::exception occurred during a call to the module ";
    detail::exceptionContext(md_, p, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n" << e.what();
    throw *cached_exception_;
  }
  catch(std::string const& s) {
    counts.template increment<stats::ExceptionThrown>();
    state_ = Exception;
    cached_exception_ = std::make_shared<art::Exception>(errors::BadExceptionType, "std::string");
    *cached_exception_
      << "A std::string thrown as an exception occurred during a call to the module ";
    detail::exceptionContext(md_, p, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n string = " << s;
    throw *cached_exception_;
  }
  catch(char const* c) {
    counts.template increment<stats::ExceptionThrown>();
    state_ = Exception;
    cached_exception_ = std::make_shared<art::Exception>(errors::BadExceptionType, "const char *");
    *cached_exception_
      << "A const char* thrown as an exception occurred during a call to the module ";
    detail::exceptionContext(md_, p, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n const char* = " << c << "\n";
    throw *cached_exception_;
  }
  catch(...) {
    counts.template increment<stats::ExceptionThrown>();
    state_ = Exception;
    cached_exception_ = std::make_shared<art::Exception>(errors::Unknown, "repeated");
    *cached_exception_
      << "An unknown occurred during a previous call to the module ";
    detail::exceptionContext(md_, p, *cached_exception_) << "and cannot be repropagated.\n";
    throw *cached_exception_;
  }

  return rc;
}
#endif /* art_Framework_Principal_Worker_h */

// Local Variables:
// mode: c++
// End:
