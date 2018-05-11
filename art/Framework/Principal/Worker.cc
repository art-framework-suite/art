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
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "hep_concurrency/tsan.h"
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
    delete md_.load();
    md_ = nullptr;
    delete cached_exception_.load();
    cached_exception_ = nullptr;
    delete waitingTasks_.load();
    waitingTasks_ = nullptr;
  }

  Worker::Worker(ModuleDescription const& md, WorkerParams const& wp)
  {
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec
          << " name: " << md.moduleName() << " label: " << md.moduleLabel();
      TDEBUG_FUNC_SI_MSG(5, "Worker ctor", wp.scheduleID_, msg.str());
    }
    md_ = new ModuleDescription(md);
    actions_ = &wp.actions_;
    actReg_ = &wp.actReg_;
    state_ = Ready;
    cached_exception_ = new exception_ptr;
    workStarted_ = false;
    returnCode_ = false;
    waitingTasks_ = new WaitingTaskList;
    counts_visited_ = 0;
    counts_run_ = 0;
    counts_passed_ = 0;
    counts_failed_ = 0;
    counts_thrown_ = 0;
  }

  ModuleDescription const&
  Worker::description() const
  {
    return *md_.load();
  }

  ModuleDescription const*
  Worker::descPtr() const
  {
    return md_.load();
  }

  string const&
  Worker::label() const
  {
    return md_.load()->moduleLabel();
  }

  // Used only by WorkerInPath.
  bool
  Worker::returnCode(ScheduleID const /*sid*/) const
  {
    return returnCode_.load();
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
  Worker::reset(ScheduleID const sid)
  {
    state_ = Ready;
    delete cached_exception_.load();
    cached_exception_ = new exception_ptr;
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec
          << " Resetting waitingTasks_";
      TDEBUG_FUNC_SI_MSG(6, "Worker::reset", sid, msg.str());
    }
    waitingTasks_.load()->reset();
    workStarted_ = false;
    returnCode_ = false;
  }

  // Used only by writeSummary
  size_t
  Worker::timesVisited() const
  {
    return counts_visited_.load();
  }

  // Used only by writeSummary
  size_t
  Worker::timesRun() const
  {
    return counts_run_.load();
  }

  // Used only by writeSummary
  size_t
  Worker::timesPassed() const
  {
    return counts_passed_.load();
  }

  // Used only by writeSummary
  size_t
  Worker::timesFailed() const
  {
    return counts_failed_.load();
  }

  // Used only by writeSummary
  size_t
  Worker::timesExcept() const
  {
    return counts_thrown_.load();
  }

  void
  Worker::beginJob()
  {
    try {
      actReg_.load()->sPreModuleBeginJob.invoke(*md_.load());
      implBeginJob();
      actReg_.load()->sPostModuleBeginJob.invoke(*md_.load());
    }
    catch (cet::exception& e) {
      LogError("BeginJob") << "A cet::exception is going through "
                           << workerType() << ":\n";
      e << "A cet::exception is going through " << workerType() << ":\n"
        << *md_.load();
      throw Exception(errors::OtherArt, string(), e);
    }
    catch (bad_alloc& e) {
      LogError("BeginJob") << "A bad_alloc is going through " << workerType()
                           << ":\n"
                           << *md_.load() << "\n";
      throw;
    }
    catch (exception& e) {
      LogError("BeginJob") << "A exception is going through " << workerType()
                           << ":\n"
                           << *md_.load() << "\n";
      throw Exception(errors::StdException)
        << "A exception is going through " << workerType() << ":\n"
        << *md_.load() << "\n";
    }
    catch (string& s) {
      LogError("BeginJob") << "module caught an string during beginJob\n";
      throw Exception(errors::BadExceptionType) << "string = " << s << "\n"
                                                << *md_.load() << "\n";
    }
    catch (char const* c) {
      LogError("BeginJob") << "module caught an const char* during beginJob\n";
      throw Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                                << *md_.load();
    }
    catch (...) {
      LogError("BeginJob") << "An unknown Exception occurred in\n"
                           << *md_.load() << "\n";
      throw Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                       << *md_.load() << "\n";
    }
  }

  void
  Worker::endJob()
  {
    try {
      actReg_.load()->sPreModuleEndJob.invoke(*md_.load());
      implEndJob();
      actReg_.load()->sPostModuleEndJob.invoke(*md_.load());
    }
    catch (cet::exception& e) {
      LogError("EndJob") << "A cet::exception is going through " << workerType()
                         << ":\n";
      e << "A cet::exception is going through " << workerType() << ":\n"
        << *md_.load();
      throw Exception(errors::OtherArt, string(), e);
    }
    catch (bad_alloc& e) {
      LogError("EndJob") << "A bad_alloc is going through " << workerType()
                         << ":\n"
                         << *md_.load() << "\n";
      throw;
    }
    catch (exception& e) {
      LogError("EndJob") << "An exception is going through " << workerType()
                         << ":\n"
                         << *md_.load() << "\n";
      throw Exception(errors::StdException)
        << "A exception is going through " << workerType() << ":\n"
        << *md_.load() << "\n"
        << e.what();
    }
    catch (string& s) {
      LogError("EndJob") << "module caught an string during endJob\n";
      throw Exception(errors::BadExceptionType) << "string = " << s << "\n"
                                                << *md_.load() << "\n";
    }
    catch (char const* c) {
      LogError("EndJob") << "module caught an const char* during endJob\n";
      throw Exception(errors::BadExceptionType) << "cstring = " << c << "\n"
                                                << *md_.load() << "\n";
    }
    catch (...) {
      LogError("EndJob") << "An unknown Exception occurred in\n"
                         << *md_.load() << "\n";
      throw Exception(errors::Unknown) << "An unknown Exception occurred in\n"
                                       << *md_.load() << "\n";
    }
  }

  void
  Worker::respondToOpenInputFile(FileBlock const& fb)
  {
    actReg_.load()->sPreModuleRespondToOpenInputFile.invoke(*md_.load());
    implRespondToOpenInputFile(fb);
    actReg_.load()->sPostModuleRespondToOpenInputFile.invoke(*md_.load());
  }

  void
  Worker::respondToCloseInputFile(FileBlock const& fb)
  {
    actReg_.load()->sPreModuleRespondToCloseInputFile.invoke(*md_.load());
    implRespondToCloseInputFile(fb);
    actReg_.load()->sPostModuleRespondToCloseInputFile.invoke(*md_.load());
  }

  void
  Worker::respondToOpenOutputFiles(FileBlock const& fb)
  {
    actReg_.load()->sPreModuleRespondToOpenOutputFiles.invoke(*md_.load());
    implRespondToOpenOutputFiles(fb);
    actReg_.load()->sPostModuleRespondToOpenOutputFiles.invoke(*md_.load());
  }

  void
  Worker::respondToCloseOutputFiles(FileBlock const& fb)
  {
    actReg_.load()->sPreModuleRespondToCloseOutputFiles.invoke(*md_.load());
    implRespondToCloseOutputFiles(fb);
    actReg_.load()->sPostModuleRespondToCloseOutputFiles.invoke(*md_.load());
  }

  bool
  Worker::doWork(Transition const trans,
                 Principal& principal,
                 PathContext const& pc)
  {
    switch (state_.load()) {
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
        mf::LogWarning("repeat") << "A module has been invoked a second time "
                                    "even though it caught an exception during "
                                    "the previous invocation.\nThis may be an "
                                    "indication of a configuration problem.\n";
        rethrow_exception(*cached_exception_.load());
      }
      case Working:
        break; // See below.
    }
    bool rc = false;
    try {
      if (state_.load() == Working) {
        // Not part of the switch statement above because we want the
        // exception to be caught by our handling mechanism.
        throw art::Exception(errors::ScheduleExecutionFailure)
          << "A Module has been invoked while it is still being executed.\n"
          << "Product dependencies have invoked a module execution cycle.\n";
      }
      state_ = Working;
      ModuleContext const mc{pc, *md_.load()};
      if (trans == Transition::BeginRun) {
        actReg_.load()->sPreModuleBeginRun.invoke(mc);
        rc = implDoBegin(dynamic_cast<RunPrincipal&>(principal), mc);
        actReg_.load()->sPostModuleBeginRun.invoke(mc);
      } else if (trans == Transition::EndRun) {
        actReg_.load()->sPreModuleEndRun.invoke(mc);
        rc = implDoEnd(dynamic_cast<RunPrincipal&>(principal), mc);
        actReg_.load()->sPostModuleEndRun.invoke(mc);
      } else if (trans == Transition::BeginSubRun) {
        actReg_.load()->sPreModuleBeginSubRun.invoke(mc);
        rc = implDoBegin(dynamic_cast<SubRunPrincipal&>(principal), mc);
        actReg_.load()->sPostModuleBeginSubRun.invoke(mc);
      } else if (trans == Transition::EndSubRun) {
        actReg_.load()->sPreModuleEndSubRun.invoke(mc);
        rc = implDoEnd(dynamic_cast<SubRunPrincipal&>(principal), mc);
        actReg_.load()->sPostModuleEndSubRun.invoke(mc);
      }
      state_ = Pass;
    }
    catch (cet::exception& e) {
      state_ = ExceptionThrown;
      e << "cet::exception going through module ";
      detail::exceptionContext(*md_.load(), principal, e);
      if (auto edmEx = dynamic_cast<art::Exception*>(&e)) {
        *cached_exception_.load() = std::make_exception_ptr(*edmEx);
      } else {
        auto art_ex = art::Exception{errors::OtherArt, std::string(), e};
        *cached_exception_.load() = std::make_exception_ptr(art_ex);
      }
      throw;
    }
    catch (std::bad_alloc const& bda) {
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), principal, art_ex)
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      rethrow_exception(*cached_exception_.load());
    }
    catch (std::exception const& e) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::StdException}
                    << "A exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n"
        << e.what();
      rethrow_exception(*cached_exception_.load());
    }
    catch (std::string const& s) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n string = " << s;
      rethrow_exception(*cached_exception_.load());
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "const char *"}
                    << "A const char* thrown as an exception occurred during a "
                       "call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), principal, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n const char* = " << c << "\n";
      rethrow_exception(*cached_exception_.load());
    }
    catch (...) {
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), principal, art_ex)
        << "and cannot be repropagated.\n";
      rethrow_exception(*cached_exception_.load());
    }
    return rc;
  }

  // This is used to do trigger results insertion, and to run workers
  // on the end path.
  void
  Worker::doWork_event(EventPrincipal& p,
                       ScheduleID const sid,
                       PathContext const& pc)
  {
    ++counts_visited_;
    returnCode_ = false;
    try {
      // Transition from Ready state to Working state.
      state_ = Working;
      ModuleContext const mc{pc, description()};
      actReg_.load()->sPreModule.invoke(mc);
      // Note: Only filters ever return false, and when they do it means they
      // have rejected.
      returnCode_ = implDoProcess(p, sid, mc);
      actReg_.load()->sPostModule.invoke(mc);
      if (returnCode_.load()) {
        state_ = Pass;
      } else {
        state_ = Fail;
      }
    }
    catch (cet::exception& e) {
      auto action = actions_.load()->find(e.root_cause());
      // If we are processing an endPath, treat SkipEvent or FailPath as
      // FailModule, so any subsequent OutputModules are still run.
      if (pc.isEndPath()) {
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
        detail::exceptionContext(*md_.load(), p, e);
        if (auto edmEx = dynamic_cast<Exception*>(&e)) {
          *cached_exception_.load() = make_exception_ptr(*edmEx);
        } else {
          *cached_exception_.load() =
            make_exception_ptr(Exception{errors::OtherArt, string(), e});
        }
        rethrow_exception(*cached_exception_.load());
      }
    }
    catch (bad_alloc const& bda) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      rethrow_exception(*cached_exception_.load());
    }
    catch (exception const& e) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::StdException}
                    << "A exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n"
        << e.what();
      rethrow_exception(*cached_exception_.load());
    }
    catch (string const& s) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n string = " << s;
      rethrow_exception(*cached_exception_.load());
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "const char *"}
                    << "A const char* thrown as an exception occurred during a "
                       "call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n const char* = " << c << "\n";
      rethrow_exception(*cached_exception_.load());
    }
    catch (...) {
      ++counts_thrown_;
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n";
      rethrow_exception(*cached_exception_.load());
    }
  }

  namespace {
    class RunWorkerFunctor {
    public:
      RunWorkerFunctor(Worker* w,
                       EventPrincipal& p,
                       ScheduleID const sid,
                       PathContext const& pc)
        : w_{w}, p_{p}, sid_{sid}, pc_{pc}
      {}
      void
      operator()() const
      {
        w_->runWorker(p_, sid_, pc_);
      }

    private:
      Worker* w_;
      EventPrincipal& p_;
      ScheduleID const sid_;
      PathContext const pc_; // Must own because it is transient
    };
  }

  void
  Worker::runWorker(EventPrincipal& p,
                    ScheduleID const sid,
                    PathContext const& pc)
  {
    TDEBUG_BEGIN_TASK_SI(4, "runWorker", sid);
    returnCode_ = false;
    try {
      // Transition from Ready state to Working state.
      state_ = Working;
      ModuleContext const mc{pc, description()};
      actReg_.load()->sPreModule.invoke(mc);
      // Note: Only filters ever return false, and when they do it
      // means they have rejected.
      returnCode_ = implDoProcess(p, sid, mc);
      actReg_.load()->sPostModule.invoke(mc);
      state_ = Fail;
      if (returnCode_.load()) {
        state_ = Pass;
      }
    }
    catch (cet::exception& e) {
      auto action = actions_.load()->find(e.root_cause());
      // If we are processing an endPath, treat SkipEvent or FailPath
      // as FailModule, so any subsequent OutputModules are still run.
      if (pc.isEndPath()) {
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
        if (auto edmEx = dynamic_cast<Exception*>(&e)) {
          *cached_exception_.load() = make_exception_ptr(*edmEx);
        } else {
          *cached_exception_.load() =
            make_exception_ptr(Exception{errors::OtherArt, string(), e});
        }
        waitingTasks_.load()->doneWaiting(*cached_exception_.load());
        TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
        return;
      }
    }
    catch (bad_alloc const& bda) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      waitingTasks_.load()->doneWaiting(*cached_exception_.load());
      TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
      return;
    }
    catch (exception const& e) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::StdException}
                    << "A exception occurred during a call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n"
        << e.what();
      waitingTasks_.load()->doneWaiting(*cached_exception_.load());
      TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
      return;
    }
    catch (string const& s) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n string = " << s;
      waitingTasks_.load()->doneWaiting(*cached_exception_.load());
      TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
      return;
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "const char *"}
                    << "A const char* thrown as an exception occurred during a "
                       "call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n"
        << "Previous information:\n const char* = " << c << "\n";
      waitingTasks_.load()->doneWaiting(*cached_exception_.load());
      TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
      return;
    }
    catch (...) {
      ++counts_thrown_;
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module ";
      *cached_exception_.load() = make_exception_ptr(art_ex);
      detail::exceptionContext(*md_.load(), p, art_ex)
        << "and cannot be repropagated.\n";
      waitingTasks_.load()->doneWaiting(*cached_exception_.load());
      TDEBUG_END_TASK_SI_ERR(4, "runWorker", sid, "because of EXCEPTION");
      return;
    }
    waitingTasks_.load()->doneWaiting(exception_ptr{});
    TDEBUG_END_TASK_SI(4, "runWorker", sid);
    return;
  }

  void
  Worker::doWork_event(WaitingTask* workerInPathDoneTask,
                       EventPrincipal& p,
                       ScheduleID const sid,
                       PathContext const& pc)
  {
    TDEBUG_BEGIN_FUNC_SI(4, "Worker::doWork_event", sid);
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
    waitingTasks_.load()->add(workerInPathDoneTask);
    ++counts_visited_;
    bool expected = false;
    if (workStarted_.compare_exchange_strong(expected, true)) {
      RunWorkerFunctor runWorkerFunctor{this, p, sid, pc};
      auto chain = serialTaskQueueChain();
      if (chain) {
        // Must be a serialized shared module (including legacy).
        {
          ostringstream msg;
          msg << "pushing onto chain " << hex << ((unsigned long*)chain) << dec;
          TDEBUG_FUNC_SI_MSG(4, "Worker::doWork_event", sid, msg.str());
        }
        chain->push(runWorkerFunctor);
        TDEBUG_END_FUNC_SI(4, "Worker::doWork_event", sid);
        return;
      }
      // Must be a replicated or shared module with no serialization.
      TDEBUG_FUNC_SI_MSG(
        4, "Worker::doWork_event", sid, "calling worker functor");
      runWorkerFunctor();
      TDEBUG_END_FUNC_SI(4, "Worker::doWork_event", sid);
      return;
    }
    // Worker is running on another path, exit without running the waiting
    // worker done tasks.
    TDEBUG_END_FUNC_SI_ERR(4,
                           "Worker::doWork_event",
                           sid,
                           "work already in progress on another path");
  }

} // namespace art
