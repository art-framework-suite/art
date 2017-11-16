#include "art/Framework/Principal/Worker.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/BranchActionType.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/MaybeIncrementCounts.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/DebugMacros.h"
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
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

using namespace hep::concurrency;
using namespace std;

using mf::LogError;

namespace art {

  namespace detail {

    cet::exception&
    exceptionContext(ModuleDescription const& md,
                     Principal const& principal,
                     cet::exception& e)
    {
      e << md.moduleName() << "/" << md.moduleLabel() << " ";
      auto bt = principal.branchType();
      if (bt == InRun) {
        e << principal.runID();
      } else if (bt == InSubRun) {
        e << principal.subRunID();
      } else if (bt == InEvent) {
        e << principal.eventID();
      }
      e << "\n";
      return e;
    }

  } // namespace detail

  Worker::~Worker() noexcept
  {
    // TDEBUG(5) << "Worker dtor: 0x" << hex << ((unsigned long)this) << dec <<
    // "\n";
  }

  Worker::Worker(ModuleDescription const& md, WorkerParams const& wp)
    : md_{md}
    , actions_{wp.actions_}
    , actReg_{wp.actReg_}
    , moduleThreadingType_{wp.moduleThreadingType_}
    , state_{Ready}
    , cached_exception_{}
    , workStarted_{false}
    , returnCode_{false}
    , waitingTasks_{}
    , counts_visited_{}
    , counts_run_{}
    , counts_passed_{}
    , counts_failed_{}
    , counts_thrown_{}
  {
    TDEBUG(5) << "Worker ctor: 0x" << hex << ((unsigned long)this) << dec
              << " (" << wp.streamIndex_ << ")"
              << " name: " << md.moduleName() << " label: " << md.moduleLabel()
              << "\n";
  }

  ModuleDescription const&
  Worker::description() const
  {
    return md_;
  }

  ModuleDescription const*
  Worker::descPtr() const
  {
    return &md_;
  }

  string const&
  Worker::label() const
  {
    return md_.moduleLabel();
  }

  // Used only by WorkerInPath.
  bool
  Worker::returnCode(int /*si*/) const
  {
    return returnCode_;
  }

  SerialTaskQueueChain*
  Worker::serialTaskQueueChain() const
  {
    return implSerialTaskQueueChain();
  }

  // Used by EventProcessor
  // Used by Schedule
  // Used by EndPathExecutor
  void
  Worker::reset(int si)
  {
    state_ = Ready;
    cached_exception_ = exception_ptr{};
    {
      ostringstream buf;
      buf << "-----> Worker::reset: 0x" << hex << ((unsigned long)this) << dec
          << " Resetting waitingTasks_ (" << si << ") ...\n";
      TDEBUG(6) << buf.str();
    }
    waitingTasks_.reset();
    workStarted_ = false;
    returnCode_ = false;
  }

  // Used only by writeSummary
  size_t
  Worker::timesVisited() const
  {
    // return counts_.times<stats::Visited>();
    return counts_visited_;
  }

  // Used only by writeSummary
  size_t
  Worker::timesRun() const
  {
    // return counts_.times<stats::Run>();
    return counts_run_;
  }

  // Used only by writeSummary
  size_t
  Worker::timesPassed() const
  {
    // return counts_.times<stats::Passed>();
    return counts_passed_;
  }

  // Used only by writeSummary
  size_t
  Worker::timesFailed() const
  {
    // return counts_.times<stats::Failed>();
    return counts_failed_;
  }

  // Used only by writeSummary
  size_t
  Worker::timesExcept() const
  {
    // return counts_.times<stats::ExceptionThrown>();
    return counts_thrown_;
  }

  void
  Worker::beginJob() try {
    actReg_.sPreModuleBeginJob.invoke(md_);
    implBeginJob();
    actReg_.sPostModuleBeginJob.invoke(md_);
  }
  catch (cet::exception& e) {
    // should event id be included?
    LogError("BeginJob") << "A cet::exception is going through " << workerType()
                         << ":\n";
    e << "A cet::exception is going through " << workerType() << ":\n" << md_;
    throw Exception(errors::OtherArt, string(), e);
  }
  catch (bad_alloc& e) {
    LogError("BeginJob") << "A bad_alloc is going through " << workerType()
                         << ":\n"
                         << md_ << "\n";
    throw;
  }
  catch (exception& e) {
    LogError("BeginJob") << "A exception is going through " << workerType()
                         << ":\n"
                         << md_ << "\n";
    throw Exception(errors::StdException)
      << "A exception is going through " << workerType() << ":\n"
      << md_ << "\n";
  }
  catch (string& s) {
    LogError("BeginJob") << "module caught an string during beginJob\n";
    throw Exception(errors::BadExceptionType) << "string = " << s << "\n"
                                              << md_ << "\n";
  }
  catch (char const* c) {
    LogError("BeginJob") << "module caught an const char* during beginJob\n";
    throw Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                              << md_;
  }
  catch (...) {
    LogError("BeginJob") << "An unknown Exception occurred in\n" << md_ << "\n";
    throw Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                     << md_ << "\n";
  }

  void
  Worker::endJob() try {
    actReg_.sPreModuleEndJob.invoke(md_);
    implEndJob();
    actReg_.sPostModuleEndJob.invoke(md_);
  }
  catch (cet::exception& e) {
    LogError("EndJob") << "A cet::exception is going through " << workerType()
                       << ":\n";
    // should event id be included?
    e << "A cet::exception is going through " << workerType() << ":\n" << md_;
    throw Exception(errors::OtherArt, string(), e);
  }
  catch (bad_alloc& e) {
    LogError("EndJob") << "A bad_alloc is going through " << workerType()
                       << ":\n"
                       << md_ << "\n";
    throw;
  }
  catch (exception& e) {
    LogError("EndJob") << "An exception is going through " << workerType()
                       << ":\n"
                       << md_ << "\n";
    throw Exception(errors::StdException)
      << "A exception is going through " << workerType() << ":\n"
      << md_ << "\n"
      << e.what();
  }
  catch (string& s) {
    LogError("EndJob") << "module caught an string during endJob\n";
    throw Exception(errors::BadExceptionType) << "string = " << s << "\n"
                                              << md_ << "\n";
  }
  catch (char const* c) {
    LogError("EndJob") << "module caught an const char* during endJob\n";
    throw Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                              << md_ << "\n";
  }
  catch (...) {
    LogError("EndJob") << "An unknown Exception occurred in\n" << md_ << "\n";
    throw Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                     << md_ << "\n";
  }

  void
  Worker::respondToOpenInputFile(FileBlock const& fb)
  {
    actReg_.sPreModuleRespondToOpenInputFile.invoke(md_);
    implRespondToOpenInputFile(fb);
    actReg_.sPostModuleRespondToOpenInputFile.invoke(md_);
  }

  void
  Worker::respondToCloseInputFile(FileBlock const& fb)
  {
    actReg_.sPreModuleRespondToCloseInputFile.invoke(md_);
    implRespondToCloseInputFile(fb);
    actReg_.sPostModuleRespondToCloseInputFile.invoke(md_);
  }

  void
  Worker::respondToOpenOutputFiles(FileBlock const& fb)
  {
    actReg_.sPreModuleRespondToOpenOutputFiles.invoke(md_);
    implRespondToOpenOutputFiles(fb);
    actReg_.sPostModuleRespondToOpenOutputFiles.invoke(md_);
  }

  void
  Worker::respondToCloseOutputFiles(FileBlock const& fb)
  {
    actReg_.sPreModuleRespondToCloseOutputFiles.invoke(md_);
    implRespondToCloseOutputFiles(fb);
    actReg_.sPostModuleRespondToCloseOutputFiles.invoke(md_);
  }

  bool
  Worker::doWork(Transition trans,
                 Principal& principal,
                 CurrentProcessingContext* cpc)
  {
    switch (state_) {
      case Ready:
        break;
      case Pass:
        return true;
      case Fail:
        return false;
      case ExceptionThrown: {
        // Rethrow the cached exception again. It seems impossible to
        // get here a second time unless a cet::exception has been
        // thrown previously.
        mf::LogWarning("repeat")
          << "A module has been invoked a second time even though"
             " it caught an exception during the previous invocation."
             "\nThis may be an indication of a configuration problem.\n";
        rethrow_exception(cached_exception_);
      }
      case Working:
        break; // See below.
    }
    bool rc = false;
    try {
      if (state_ == Working) {
        // Not part of the switch statement above because we want the
        // exception to be caught by our handling mechanism.
        throw art::Exception(errors::ScheduleExecutionFailure)
          << "A Module has been invoked while it is still being executed.\n"
          << "Product dependencies have invoked a module execution cycle.\n";
      }
      state_ = Working;
      detail::CPCSentry sentry{*cpc};
      if (trans == Transition::BeginRun) {
        actReg_.sPreModuleBeginRun.invoke(md_);
        rc = implDoBegin(dynamic_cast<RunPrincipal&>(principal), cpc);
        actReg_.sPostModuleBeginRun.invoke(md_);
      } else if (trans == Transition::EndRun) {
        actReg_.sPreModuleEndRun.invoke(md_);
        rc = implDoEnd(dynamic_cast<RunPrincipal&>(principal), cpc);
        actReg_.sPostModuleEndRun.invoke(md_);
      } else if (trans == Transition::BeginSubRun) {
        actReg_.sPreModuleBeginSubRun.invoke(md_);
        rc = implDoBegin(dynamic_cast<SubRunPrincipal&>(principal), cpc);
        actReg_.sPostModuleBeginSubRun.invoke(md_);
      } else if (trans == Transition::EndSubRun) {
        actReg_.sPreModuleEndSubRun.invoke(md_);
        rc = implDoEnd(dynamic_cast<SubRunPrincipal&>(principal), cpc);
        actReg_.sPostModuleEndSubRun.invoke(md_);
      }
      state_ = Pass;
    }
    catch (cet::exception& e) {
      state_ = ExceptionThrown;
      e << "cet::exception going through module ";
      detail::exceptionContext(md_, principal, e);
      if (auto edmEx = dynamic_cast<art::Exception*>(&e)) {
        cached_exception_ = std::make_exception_ptr(*edmEx);
      } else {
        auto art_ex = art::Exception{errors::OtherArt, std::string(), e};
        cached_exception_ = std::make_exception_ptr(art_ex);
      }
      throw;
    }
    catch (std::bad_alloc const& bda) {
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception occurred during a call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, principal, art_ex)
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      rethrow_exception(cached_exception_);
    }
    catch (std::exception const& e) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::StdException}
                    << "A exception occurred during a call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n"
        << e.what();
      rethrow_exception(cached_exception_);
    }
    catch (std::string const& s) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n string = " << s;
      rethrow_exception(cached_exception_);
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "const char *"}
                    << "A const char* thrown as an exception occurred during a "
                       "call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n const char* = " << c << "\n";
      rethrow_exception(cached_exception_);
    }
    catch (...) {
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, principal, art_ex)
        << "and cannot be repropagated.\n";
      rethrow_exception(cached_exception_);
    }
    return rc;
  }

  // This is used to do trigger results insertion,
  // and to run workers on the end path.
  void
  Worker::doWork_event(EventPrincipal& p, int si, CurrentProcessingContext* cpc)
  {
    ++counts_visited_;
    returnCode_ = false;
    try {
      // Transition from Ready state to Working state.
      state_ = Working;
      detail::CPCSentry sentry{*cpc};
      actReg_.sPreModule.invoke(md_);
      // Note: Only filters ever return false, and when they do it means they
      // have rejected.
      returnCode_ = implDoProcess(p, si, cpc);
      // FIXME: We construct the following context only so the correct
      // streamIndex is provided within services.
      CurrentProcessingContext cpc{si, nullptr, -1, false};
      detail::CPCSentry sentry2{cpc};
      actReg_.sPostModule.invoke(md_);
      if (returnCode_) {
        state_ = Pass;
      } else {
        state_ = Fail;
      }
    }
    catch (cet::exception& e) {
      auto action = actions_.find(e.root_cause());
      // If we are processing an endPath, treat SkipEvent or FailPath as
      // FailModule, so any subsequent OutputModules are still run.
      if (cpc->isEndPath()) {
        if ((action == actions::SkipEvent) || (action == actions::FailPath)) {
          action = actions::FailModule;
        }
      }
      if (action == actions::IgnoreCompletely) {
        state_ = Pass;
        returnCode_ = true;
        ++counts_passed_;
        mf::LogWarning("IgnoreCompletely") << "Module ignored an exception\n"
                                           << e.what() << "\n";
        // WARNING: We will continue execution below!!!
      } else if (action == actions::FailModule) {
        state_ = Fail;
        returnCode_ = true;
        ++counts_failed_;
        mf::LogWarning("FailModule") << "Module failed due to an exception\n"
                                     << e.what() << "\n";
        // WARNING: We will continue execution below!!!
      } else {
        state_ = ExceptionThrown;
        ++counts_thrown_;
        e << "cet::exception going through module ";
        detail::exceptionContext(md_, p, e);
        if (auto edmEx = dynamic_cast<Exception*>(&e)) {
          cached_exception_ = make_exception_ptr(*edmEx);
        } else {
          cached_exception_ =
            make_exception_ptr(Exception{errors::OtherArt, string(), e});
        }
        rethrow_exception(cached_exception_);
      }
    }
    catch (bad_alloc const& bda) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception occurred during a call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, p, art_ex)
        << "The job has probably "
           "exhausted the virtual memory "
           "available to the process.\n";
      rethrow_exception(cached_exception_);
    }
    catch (exception const& e) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::StdException}
                    << "A exception occurred during a call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n"
        << e.what();
      rethrow_exception(cached_exception_);
    }
    catch (string const& s) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n string = " << s;
      rethrow_exception(cached_exception_);
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "const char *"}
                    << "A const char* thrown as an exception occurred during a "
                       "call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n const char* = " << c << "\n";
      rethrow_exception(cached_exception_);
    }
    catch (...) {
      ++counts_thrown_;
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module ";
      cached_exception_ = make_exception_ptr(art_ex);
      detail::exceptionContext(md_, p, art_ex)
        << "and cannot be repropagated.\n";
      rethrow_exception(cached_exception_);
    }
  }

  void
  Worker::doWork_event(WaitingTask* workerInPathDoneTask,
                       EventPrincipal& p,
                       int si,
                       CurrentProcessingContext* cpc)
  {
    TDEBUG(4) << "-----> Begin Worker::doWork_event (" << si << ") ...\n";
    // Note: We actually can have more than one entry in this
    // list because a worker may be one more than one path,
    // and if both paths are running in parallel, then it is
    // possible that they both attempt to run the same worker
    // at nearly the same time.  We arrange so that the worker
    // itself only runs once, but we do have to notify all the
    // paths that the worker has finished, hence the waiting
    // list of notification tasks.
    // Note: threading: More than one task can enter here in
    // the case that paths running in parallel share the same
    // worker.
    waitingTasks_.add(workerInPathDoneTask);
    ++counts_visited_;
    bool expected = false;
    if (workStarted_.compare_exchange_strong(expected, true)) {
      // There is a hilarious bug in gcc 6.3 that allows a non-mutable
      // lambda expression to modify the class captured by this.  It
      // declares the this pointer inside the lambda as:
      //      T* const
      // instead of:
      //      T const*
      // like it should!!!
      // The SerialTaskQueueChain gets mad if the lambda is mutable
      // because it declares the functor argument to push() as
      // T const& and gcc 6.3 does get that part right.
      // So to work around the const in the SerialTaskQueueChain
      // declaration we make the lambda not mutable, and to allow
      // future compilers which fix the misdeclaration of the this
      // pointer to work, we use the sledgehammer and const_cast
      // the this pointer so we can modify returnCode_ from the
      // non-mutable lambda.  Jeesh.
      auto runWorkerFunctor = [this, &p, si, cpc]() {
        TDEBUG(4) << "=====> Begin runWorkerFunctor (" << si << ") ...\n";
        auto This = const_cast<Worker*>(this);
        This->returnCode_ = false;
        try {
          // Transition from Ready state to Working state.
          This->state_ = Working;
          detail::CPCSentry sentry{*cpc};
          actReg_.sPreModule.invoke(md_);
          // Note: Only filters ever return false, and when they do it means
          // they have rejected.
          This->returnCode_ = implDoProcess(p, si, cpc);
          actReg_.sPostModule.invoke(md_);
          if (returnCode_) {
            This->state_ = Pass;
          } else {
            This->state_ = Fail;
          }
        }
        catch (cet::exception& e) {
          auto action = actions_.find(e.root_cause());
          // If we are processing an endPath, treat SkipEvent or FailPath as
          // FailModule, so any subsequent OutputModules are still run.
          if (cpc->isEndPath()) {
            if ((action == actions::SkipEvent) ||
                (action == actions::FailPath)) {
              action = actions::FailModule;
            }
          }
          if (action == actions::IgnoreCompletely) {
            This->state_ = Pass;
            This->returnCode_ = true;
            ++This->counts_passed_;
            mf::LogWarning("IgnoreCompletely")
              << "Module ignored an exception\n"
              << e.what() << "\n";
            // WARNING: We will continue execution below!!!
          } else if (action == actions::FailModule) {
            This->state_ = Fail;
            This->returnCode_ = true;
            ++This->counts_failed_;
            mf::LogWarning("FailModule")
              << "Module failed due to an exception\n"
              << e.what() << "\n";
            // WARNING: We will continue execution below!!!
          } else {
            This->state_ = ExceptionThrown;
            ++This->counts_thrown_;
            e << "cet::exception going through module ";
            if (auto edmEx = dynamic_cast<Exception*>(&e)) {
              This->cached_exception_ = make_exception_ptr(*edmEx);
            } else {
              This->cached_exception_ =
                make_exception_ptr(Exception{errors::OtherArt, string(), e});
            }
            This->waitingTasks_.doneWaiting(cached_exception_);
            TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                      << ") ... because of exception\n";
            return;
          }
        }
        catch (bad_alloc const& bda) {
          This->state_ = ExceptionThrown;
          ++This->counts_thrown_;
          auto art_ex =
            Exception{errors::BadAlloc}
            << "A bad_alloc exception occurred during a call to the module ";
          This->cached_exception_ = make_exception_ptr(art_ex);
          detail::exceptionContext(md_, p, art_ex)
            << "The job has probably exhausted the virtual memory available to "
               "the process.\n";
          This->waitingTasks_.doneWaiting(cached_exception_);
          TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                    << ") ... because of exception\n";
          return;
        }
        catch (exception const& e) {
          This->state_ = ExceptionThrown;
          ++This->counts_thrown_;
          auto art_ex = Exception{errors::StdException}
                        << "A exception occurred during a call to the module ";
          This->cached_exception_ = make_exception_ptr(art_ex);
          detail::exceptionContext(md_, p, art_ex)
            << "and cannot be repropagated.\n"
            << "Previous information:\n"
            << e.what();
          This->waitingTasks_.doneWaiting(cached_exception_);
          TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                    << ") ... because of exception\n";
          return;
        }
        catch (string const& s) {
          This->state_ = ExceptionThrown;
          ++This->counts_thrown_;
          auto art_ex = Exception{errors::BadExceptionType, "string"}
                        << "A string thrown as an exception occurred during a "
                           "call to the module ";
          This->cached_exception_ = make_exception_ptr(art_ex);
          detail::exceptionContext(md_, p, art_ex)
            << "and cannot be repropagated.\n"
            << "Previous information:\n string = " << s;
          This->waitingTasks_.doneWaiting(cached_exception_);
          TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                    << ") ... because of exception\n";
          return;
        }
        catch (char const* c) {
          This->state_ = ExceptionThrown;
          ++This->counts_thrown_;
          auto art_ex =
            Exception{errors::BadExceptionType, "const char *"}
            << "A const char* thrown as an exception occurred during "
               "a call to the module ";
          This->cached_exception_ = make_exception_ptr(art_ex);
          detail::exceptionContext(md_, p, art_ex)
            << "and cannot be repropagated.\n"
            << "Previous information:\n const char* = " << c << "\n";
          This->waitingTasks_.doneWaiting(cached_exception_);
          TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                    << ") ... because of exception\n";
          return;
        }
        catch (...) {
          ++This->counts_thrown_;
          This->state_ = ExceptionThrown;
          auto art_ex =
            Exception{errors::Unknown, "repeated"}
            << "An unknown occurred during a previous call to the module ";
          This->cached_exception_ = make_exception_ptr(art_ex);
          detail::exceptionContext(md_, p, art_ex)
            << "and cannot be repropagated.\n";
          This->waitingTasks_.doneWaiting(cached_exception_);
          TDEBUG(4) << "=====> End   runWorkerFunctor (" << si
                    << ") ... because of exception\n";
          return;
        }
        This->waitingTasks_.doneWaiting(exception_ptr{});
        TDEBUG(4) << "=====> End   runWorkerFunctor (" << si << ") ...\n";
        return;
      };
      auto chain = serialTaskQueueChain();
      if (chain) {
        // Must be a legacy or a one module.
        TDEBUG(4) << "-----> Worker::doWork_event: si: " << si
                  << " pushing onto chain " << hex << ((unsigned long*)chain)
                  << dec << "\n";
        chain->push(runWorkerFunctor);
        TDEBUG(4) << "-----> End   Worker::doWork_event (" << si << ") ...\n";
        return;
      }
      // Must be a stream or global module.
      TDEBUG(4) << "-----> Worker::doWork_event: si: " << si
                << " calling worker functor\n";
      runWorkerFunctor();
      TDEBUG(4) << "-----> End   Worker::doWork_event (" << si << ") ...\n";
      return;
    }
    // Worker is running on another path, exit without
    // running the waiting worker done tasks.
    TDEBUG(4) << "-----> End   Worker::doWork_event (" << si
              << ") ... work already in progress on another path\n";
  }

} // namespace art
