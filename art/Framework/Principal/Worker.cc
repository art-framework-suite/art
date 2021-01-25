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
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
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

namespace {
  std::string
  brief_context(art::ModuleDescription const& md)
  {
    return md.moduleName() + "/" + md.moduleLabel();
  }

  std::string
  brief_context(art::ModuleDescription const& md,
                art::Principal const& principal)
  {
    std::ostringstream result;
    result << brief_context(md) << ' ';
    auto const bt = principal.branchType();
    switch (bt) {
      case art::InRun:
        result << principal.runID();
        break;
      case art::InSubRun:
        result << principal.subRunID();
        break;
      case art::InEvent:
        result << principal.eventID();
        break;
      default: {}
    }
    return result.str();
  }

  [[noreturn]] void
  rethrow_with_context(std::exception_ptr eptr,
                       art::ModuleDescription const& md,
                       std::string const& transition)
  {
    using namespace art;
    assert(eptr);
    std::string const brief_module_context =
      brief_context(md) + " during " + transition;
    try {
      std::rethrow_exception(eptr);
    }
    catch (cet::exception& e) {
      throw Exception{errors::OtherArt,
                      "An exception was thrown while processing module " +
                        brief_module_context,
                      e};
    }
    catch (exception& e) {
      throw Exception{errors::StdException, brief_module_context} << e.what();
    }
    catch (string& s) {
      throw Exception{errors::BadExceptionType,
                      "A string exception was thrown while processing module " +
                        brief_module_context}
        << s << '\n';
    }
    catch (char const* c) {
      throw Exception{
        errors::BadExceptionType,
        "A char const* exception was thrown while processing module " +
          brief_module_context}
        << c << '\n';
    }
    catch (...) {
      throw Exception{errors::Unknown}
        << "An unknown exception was thrown while processing module " +
             brief_module_context;
    }
  }
}

namespace art {

  Worker::Worker(ModuleDescription const& md, WorkerParams const& wp)
    : scheduleID_{wp.scheduleID_}
    , md_{md}
    , actions_{wp.actions_}
    , actReg_{wp.actReg_}
  {
    TDEBUG_FUNC_SI(5, wp.scheduleID_)
      << hex << this << dec << " name: " << md.moduleName()
      << " label: " << md.moduleLabel();
  }

  ModuleDescription const&
  Worker::description() const
  {
    return md_;
  }

  string const&
  Worker::label() const
  {
    return md_.moduleLabel();
  }

  // Used only by WorkerInPath.
  bool
  Worker::returnCode() const
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
  Worker::reset()
  {
    state_ = Ready;
    cached_exception_ = std::exception_ptr{};
    TDEBUG_FUNC_SI(6, scheduleID_)
      << hex << this << dec << " Resetting waitingTasks_";
    waitingTasks_.reset();
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
  Worker::beginJob() try {
    actReg_.sPreModuleBeginJob.invoke(md_);
    implBeginJob();
    actReg_.sPostModuleBeginJob.invoke(md_);
  }
  catch (...) {
    rethrow_with_context(std::current_exception(), md_, "beginJob");
  }

  void
  Worker::endJob() try {
    actReg_.sPreModuleEndJob.invoke(md_);
    implEndJob();
    actReg_.sPostModuleEndJob.invoke(md_);
  }
  catch (...) {
    rethrow_with_context(std::current_exception(), md_, "endJob");
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
  Worker::doWork(Transition const trans,
                 Principal& principal,
                 ModuleContext const& mc)
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
        rethrow_exception(cached_exception_);
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
      if (trans == Transition::BeginRun) {
        actReg_.sPreModuleBeginRun.invoke(mc);
        rc = implDoBegin(dynamic_cast<RunPrincipal&>(principal), mc);
        actReg_.sPostModuleBeginRun.invoke(mc);
      } else if (trans == Transition::EndRun) {
        actReg_.sPreModuleEndRun.invoke(mc);
        rc = implDoEnd(dynamic_cast<RunPrincipal&>(principal), mc);
        actReg_.sPostModuleEndRun.invoke(mc);
      } else if (trans == Transition::BeginSubRun) {
        actReg_.sPreModuleBeginSubRun.invoke(mc);
        rc = implDoBegin(dynamic_cast<SubRunPrincipal&>(principal), mc);
        actReg_.sPostModuleBeginSubRun.invoke(mc);
      } else if (trans == Transition::EndSubRun) {
        actReg_.sPreModuleEndSubRun.invoke(mc);
        rc = implDoEnd(dynamic_cast<SubRunPrincipal&>(principal), mc);
        actReg_.sPostModuleEndSubRun.invoke(mc);
      }
      state_ = Pass;
    }
    catch (cet::exception& e) {
      state_ = ExceptionThrown;
      e << "The above exception was thrown while processing module "
        << brief_context(md_, principal) << '\n';
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
        << "A bad_alloc exception occurred during a call to the module "
        << brief_context(md_, principal) << '\n'
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (std::exception const& e) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::StdException}
                    << "An exception occurred during a call to the module "
                    << brief_context(md_, principal) << e.what();
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (std::string const& s) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module "
                    << brief_context(md_, principal) << '\n'
                    << s << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      auto art_ex = Exception{errors::BadExceptionType, "char const*"}
                    << "A char const* thrown as an exception occurred during a "
                       "call to the module "
                    << brief_context(md_, principal) << '\n'
                    << c << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (...) {
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module "
        << brief_context(md_, principal) << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    return rc;
  }

  // This is used to do trigger results insertion, and to run workers
  // on the end path.
  void
  Worker::doWork_event(EventPrincipal& p, ModuleContext const& mc)
  {
    ++counts_visited_;
    returnCode_ = false;
    try {
      // Transition from Ready state to Working state.
      state_ = Working;
      actReg_.sPreModule.invoke(mc);
      // Note: Only filters ever return false, and when they do it
      // means they have rejected.
      returnCode_ = implDoProcess(p, mc);
      actReg_.sPostModule.invoke(mc);
      if (returnCode_.load()) {
        state_ = Pass;
      } else {
        state_ = Fail;
      }
    }
    catch (cet::exception& e) {
      auto action = actions_.find(e.root_cause());
      // If we are processing an endPath, treat SkipEvent or FailPath
      // as FailModule, so any subsequent OutputModules are still run.
      if (mc.onEndPath()) {
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
        e << "The above exception was thrown while processing module "
          << brief_context(md_, p) << '\n';
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
        << "A bad_alloc exception occurred during a call to the module "
        << brief_context(md_, p) << '\n'
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (exception const& e) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::StdException}
                    << "An exception occurred during a call to the module "
                    << brief_context(md_, p) << '\n'
                    << e.what();
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (string const& s) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "string"}
                    << "A string thrown as an exception occurred during a call "
                       "to the module "
                    << brief_context(md_, p) << '\n'
                    << s << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::BadExceptionType, "char const*"}
                    << "A char const* thrown as an exception occurred during a "
                       "call to the module "
                    << brief_context(md_, p) << '\n'
                    << c << "\n";
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
    catch (...) {
      ++counts_thrown_;
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown occurred during a previous call to the module "
        << brief_context(md_, p) << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      rethrow_exception(cached_exception_);
    }
  }

  namespace {
    class RunWorkerFunctor {
    public:
      RunWorkerFunctor(Worker* w, EventPrincipal& p, ModuleContext const& mc)
        : w_{w}, p_{p}, mc_{mc}
      {}
      void
      operator()() const
      {
        w_->runWorker(p_, mc_);
      }

    private:
      Worker* w_;
      EventPrincipal& p_;
      ModuleContext const& mc_;
    };
  }

  void
  Worker::runWorker(EventPrincipal& p, ModuleContext const& mc)
  {
    auto const sid = mc.scheduleID();
    TDEBUG_BEGIN_TASK_SI(4, sid);
    returnCode_ = false;
    try {
      // Transition from Ready state to Working state.
      state_ = Working;
      actReg_.sPreModule.invoke(mc);
      // Note: Only filters ever return false, and when they do it
      // means they have rejected.
      returnCode_ = implDoProcess(p, mc);
      actReg_.sPostModule.invoke(mc);
      state_ = Fail;
      if (returnCode_.load()) {
        state_ = Pass;
      }
    }
    catch (cet::exception& e) {
      auto action = actions_.find(e.root_cause());
      // If we are processing an endPath, treat SkipEvent or FailPath
      // as FailModule, so any subsequent OutputModules are still run.
      if (mc.onEndPath()) {
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
        e << "The above exception was thrown while processing module "
          << brief_context(md_, p);
        if (auto art_ex = dynamic_cast<Exception*>(&e)) {
          cached_exception_ = make_exception_ptr(*art_ex);
        } else {
          cached_exception_ =
            make_exception_ptr(Exception{errors::OtherArt, string(), e});
        }
        waitingTasks_.doneWaiting(cached_exception_);
        TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
        return;
      }
    }
    catch (bad_alloc const& bda) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadAlloc}
        << "A bad_alloc exception was thrown while processing module "
        << brief_context(md_, p) << '\n'
        << "The job has probably exhausted the virtual memory available to the "
           "process.\n";
      cached_exception_ = make_exception_ptr(art_ex);
      waitingTasks_.doneWaiting(cached_exception_);
      TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
      return;
    }
    catch (exception const& e) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex = Exception{errors::StdException}
                    << "An exception was thrown while processing module "
                    << brief_context(md_, p) << '\n'
                    << e.what();
      cached_exception_ = make_exception_ptr(art_ex);
      waitingTasks_.doneWaiting(cached_exception_);
      TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
      return;
    }
    catch (string const& s) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadExceptionType, "string"}
        << "A string was thrown as an exception while processing module "
        << brief_context(md_, p) << '\n'
        << s << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      waitingTasks_.doneWaiting(cached_exception_);
      TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
      return;
    }
    catch (char const* c) {
      state_ = ExceptionThrown;
      ++counts_thrown_;
      auto art_ex =
        Exception{errors::BadExceptionType, "char const*"}
        << "A char const* was thrown as an exception while processing module "
        << brief_context(md_, p) << '\n'
        << c << "\n";
      cached_exception_ = make_exception_ptr(art_ex);
      waitingTasks_.doneWaiting(cached_exception_);
      TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
      return;
    }
    catch (...) {
      ++counts_thrown_;
      state_ = ExceptionThrown;
      auto art_ex =
        Exception{errors::Unknown, "repeated"}
        << "An unknown exception was thrown while processing module "
        << brief_context(md_, p) << '\n';
      cached_exception_ = make_exception_ptr(art_ex);
      waitingTasks_.doneWaiting(cached_exception_);
      TDEBUG_END_TASK_SI(4, sid) << "because of EXCEPTION";
      return;
    }
    waitingTasks_.doneWaiting(exception_ptr{});
    TDEBUG_END_TASK_SI(4, sid);
  }

  void
  Worker::doWork_event(tbb::task* workerInPathDoneTask,
                       EventPrincipal& p,
                       ModuleContext const& mc)
  {
    auto const sid = mc.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, sid);
    // Note: We actually can have more than one entry in this list
    // because a worker may be one more than one path, and if both
    // paths are running in parallel, then it is possible that they
    // both attempt to run the same worker at nearly the same time.
    // We arrange so that the worker itself only runs once, but we do
    // have to notify all the paths that the worker has finished,
    // hence the waiting list of notification tasks.
    //
    // Note: threading: More than one task can enter here in the case
    // that paths running in parallel share the same worker.
    waitingTasks_.add(workerInPathDoneTask);
    ++counts_visited_;
    bool expected = false;
    if (workStarted_.compare_exchange_strong(expected, true)) {
      RunWorkerFunctor runWorkerFunctor{this, p, mc};
      if (auto chain = serialTaskQueueChain()) {
        // Must be a serialized shared module (including legacy).
        TDEBUG_FUNC_SI(4, sid) << "pushing onto chain " << hex << chain << dec;
        chain->push(runWorkerFunctor);
        TDEBUG_END_FUNC_SI(4, sid);
        return;
      }
      // Must be a replicated or shared module with no serialization.
      TDEBUG_FUNC_SI(4, sid) << "calling worker functor";
      runWorkerFunctor();
      TDEBUG_END_FUNC_SI(4, sid);
      return;
    }
    // Worker is running on another path, exit without running the waiting
    // worker done tasks.
    TDEBUG_END_FUNC_SI(4, sid) << "work already in progress on another path";
  }

} // namespace art
