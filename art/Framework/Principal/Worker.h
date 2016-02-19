#ifndef art_Framework_Principal_Worker_h
#define art_Framework_Principal_Worker_h

// ======================================================================
/*

Worker: this is a basic scheduling unit - an abstract base class to
something that is really a producer or filter.


A worker will not actually call through to the module unless it is
in a Ready state.  After a module is actually run, the state will not
be Ready.  The Ready state can only be reestablished by doing a reset().

Pre/post module signals are posted only in the Ready state.

Execution statistics are kept here.

If a module has thrown an exception during execution, that exception
will be rethrown if the worker is entered again and the state is not Ready.
In other words, execution results (status) are cached and reused until
the worker is reset().

*/
// ======================================================================

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Services/Registry/BranchActionType.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Principal/RunStopwatch.h"
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
  // Forward-declare these here to avoid false alarms from the
  // package dependency checker.
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
  virtual ~Worker();

  virtual void reconfigure(fhicl::ParameterSet const &) = 0;

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

  std::pair<double,double> timeCpuReal() const {
      return std::pair<double,double>(timer_.cpuTime(),timer_.realTime());
  }

  void clearCounters() {
    timesRun_ = timesVisited_ = timesPassed_ = timesFailed_ = timesExcept_ = 0;
  }

  int timesRun() const { return timesRun_; }
  int timesVisited() const { return timesVisited_; }
  int timesPassed() const { return timesPassed_; }
  int timesFailed() const { return timesFailed_; }
  int timesExcept() const { return timesExcept_; }
  State state() const { return state_; }

  virtual bool modifiesEvent() const = 0;

  std::string const &label() const { return md_.moduleLabel(); }

protected:
  virtual std::string workerType() const = 0;
  virtual bool implDoBegin(EventPrincipal&,
                           CurrentProcessingContext const* cpc) = 0;
  virtual bool implDoEnd(EventPrincipal&,
                         CurrentProcessingContext const* cpc) = 0;
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
  virtual void implRespondToOpenInputFile(FileBlock const& fb) = 0;
  virtual void implRespondToCloseInputFile(FileBlock const& fb) = 0;
  virtual void implRespondToOpenOutputFiles(FileBlock const& fb) = 0;
  virtual void implRespondToCloseOutputFiles(FileBlock const& fb) = 0;

  //RunStopwatch::StopwatchPointer stopwatch_;
  cet::cpu_timer timer_;

  int timesRun_;
  int timesVisited_;
  int timesPassed_;
  int timesFailed_;
  int timesExcept_;
  State state_;

  ModuleDescription md_;
  ActionTable const & actions_;
  std::shared_ptr<art::Exception> cached_exception_; // if state is 'exception'

  cet::exempt_ptr<ActivityRegistry> actReg_;
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
}

template <typename T>
class art::detail::ModuleSignalSentry {
public:
  ModuleSignalSentry(ActivityRegistry *a, ModuleDescription& md) : a_(a), md_(&md) {
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
                 cet::exception& iEx) {
  iEx << iMD.moduleName() << "/" << iMD.moduleLabel()
      << " " << ip.id() << "\n";
  return iEx;
}

template <typename T>
bool art::Worker::doWork(typename T::MyPrincipal& ep,
                         CurrentProcessingContext const* cpc) {

  // A RunStopwatch, but only if we are processing an event.
  //std::unique_ptr<RunStopwatch> stopwatch(T::isEvent_ ? new RunStopwatch(stopwatch_) : 0);

  if (T::isEvent_) {
    ++timesVisited_;
  }
  bool rc = false;

  switch(state_) {
  case Ready: break;
  case Pass: return true;
  case Fail: return false;
  case Exception:
    {
      // rethrow the cached exception again
      // It seems impossible to
      // get here a second time until a cet::exception has been
      // thrown prviously.
      mf::LogWarning("repeat")
        << "A module has been invoked a second time even though"
        " it caught an exception during the previous invocation."
        "\nThis may be an indication of a configuration problem.\n";
      throw *cached_exception_;
    }
  case Working: break; // See below.
  }

  try {
    if (state_ == Working) {
      // Not part of the switch statement above because we want the
      // exception to be caught by our handling mechanism.
      throw art::Exception(errors::ScheduleExecutionFailure)
        << "A Module has been invoked while it is still being executed.\n"
        << "Product dependencies have invoked a module execution cycle.\n";
    }

    if (T::isEvent_) ++timesRun_;

    detail::ModuleSignalSentry<T> cpp(actReg_.get(), md_);
    state_ = Working;
    if (T::begin_) {
      rc = implDoBegin(ep, cpc);
    } else {
      rc = implDoEnd(ep, cpc);
    }

    if (rc) {
      state_ = Pass;
      if (T::isEvent_) ++timesPassed_;
    } else {
      state_ = Fail;
      if (T::isEvent_) ++timesFailed_;
    }
  }

  catch(cet::exception& e) {

    // NOTE: the warning printed as a result of ignoring or failing
    // a module will only be printed during the full true processing
    // pass of this module

    // Get the action corresponding to this exception.  However, if processing
    // something other than an event (e.g. run, subRun) always rethrow.
    actions::ActionCodes action = (T::isEvent_ ? actions_.find(e.root_cause()) : actions::Rethrow);

    // If we are processing an endpath, treat SkipEvent or FailPath
    // as FailModule, so any subsequent OutputModules are still run.
    if (cpc && cpc->isEndPath()) {
      if (action == actions::SkipEvent || action == actions::FailPath) action = actions::FailModule;
    }
    switch(action) {
    case actions::IgnoreCompletely: {
      rc=true;
      ++timesPassed_;
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
      ++timesFailed_;
      state_ = Fail;
      break;
    }

    default: {

      // we should not need to include the event/run/module names
      // the exception because the error logger will pick this
      // up automatically.  I'm leaving it in until this is
      // verified

      // here we simply add a small amount of data to the
      // exception to add some context, we could have rethrown
      // it as something else and embedded with this exception
      // as an argument to the constructor.

      if (T::isEvent_) ++timesExcept_;
      state_ = Exception;
      e << "cet::exception going through module ";
      detail::exceptionContext(md_, ep, e);
      art::Exception *edmEx = dynamic_cast<art::Exception *>(&e);
      if (edmEx) {
        cached_exception_.reset(new art::Exception(*edmEx));
      } else {
        cached_exception_.reset(new art::Exception(errors::OtherArt, std::string(), e));
      }
      throw;
    }
    }
  }

  catch(std::bad_alloc& bda) {
    if (T::isEvent_) ++timesExcept_;
    state_ = Exception;
    cached_exception_.reset(new art::Exception(errors::BadAlloc));
    *cached_exception_
      << "A std::bad_alloc exception occurred during a call to the module ";
    detail::exceptionContext(md_, ep, *cached_exception_)
      << "The job has probably exhausted the virtual memory available to the process.\n";
    throw *cached_exception_;
  }
  catch(std::exception& e) {
    if (T::isEvent_) ++timesExcept_;
    state_ = Exception;
    cached_exception_.reset(new art::Exception(errors::StdException));
    *cached_exception_
      << "A std::exception occurred during a call to the module ";
    detail::exceptionContext(md_, ep, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n" << e.what();
    throw *cached_exception_;
  }
  catch(std::string& s) {
    if (T::isEvent_) ++timesExcept_;
    state_ = Exception;
    cached_exception_.reset(new art::Exception(errors::BadExceptionType, "std::string"));
    *cached_exception_
      << "A std::string thrown as an exception occurred during a call to the module ";
    detail::exceptionContext(md_, ep, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n string = " << s;
    throw *cached_exception_;
  }
  catch(char const* c) {
    if (T::isEvent_) ++timesExcept_;
    state_ = Exception;
    cached_exception_.reset(new art::Exception(errors::BadExceptionType, "const char *"));
    *cached_exception_
      << "A const char* thrown as an exception occurred during a call to the module ";
    detail::exceptionContext(md_, ep, *cached_exception_) << "and cannot be repropagated.\n"
                                                          << "Previous information:\n const char* = " << c << "\n";
    throw *cached_exception_;
  }
  catch(...) {
    if (T::isEvent_) ++timesExcept_;
    state_ = Exception;
    cached_exception_.reset(new art::Exception(errors::Unknown, "repeated"));
    *cached_exception_
      << "An unknown occurred during a previous call to the module ";
    detail::exceptionContext(md_, ep, *cached_exception_) << "and cannot be repropagated.\n";
    throw *cached_exception_;
  }

  return rc;
}
#endif /* art_Framework_Principal_Worker_h */

// Local Variables:
// mode: c++
// End:
