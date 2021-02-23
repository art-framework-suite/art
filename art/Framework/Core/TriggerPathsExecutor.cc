#include "art/Framework/Core/TriggerPathsExecutor.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/GlobalTaskGroup.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  TriggerPathsExecutor::TriggerPathsExecutor(
    ScheduleID const scheduleID,
    PathManager& pm,
    ActionTable const& actions,
    std::unique_ptr<Worker> triggerResultsInserter,
    GlobalTaskGroup& group)
    : sc_{scheduleID}
    , actionTable_{actions}
    , triggerPathsInfo_{pm.triggerPathsInfo(scheduleID)}
    , results_inserter_{std::move(triggerResultsInserter)}
    , taskGroup_{group}
  {
    TDEBUG_FUNC_SI(5, scheduleID) << hex << this << dec;
  }

  void
  TriggerPathsExecutor::beginJob(detail::SharedResources const& resources)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.beginJob(resources);
    }
    if (results_inserter_) {
      results_inserter_->beginJob(resources);
    }
  }

  void
  TriggerPathsExecutor::endJob()
  {
    Exception error{errors::EndJobFailure};
    for (auto& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        w.endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in TriggerPathsExecutor::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in "
                 "TriggerPathsExecutor::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in TriggerPathsExecutor::endJob\n";
        throw error;
      }
    }
    if (results_inserter_) {
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        results_inserter_->endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in TriggerPathsExecutor::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in "
                 "TriggerPathsExecutor::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in TriggerPathsExecutor::endJob\n";
        throw error;
      }
    }
  }

  void
  TriggerPathsExecutor::respondToOpenInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenInputFile(fb);
    }
  }

  void
  TriggerPathsExecutor::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseInputFile(fb);
    }
  }

  void
  TriggerPathsExecutor::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenOutputFiles(fb);
    }
  }

  void
  TriggerPathsExecutor::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseOutputFiles(fb);
    }
  }

  void
  TriggerPathsExecutor::process(Transition const trans, Principal& principal)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      val.second->reset();
    }
    for (auto const& path : triggerPathsInfo_.paths()) {
      path->process(trans, principal);
    }
  }

  class TriggerPathsExecutor::PathsDoneTask {
  public:
    PathsDoneTask(TriggerPathsExecutor* const schedule,
                  WaitingTaskPtr const endPathTask,
                  EventPrincipal& principal,
                  GlobalTaskGroup& group)
      : schedule_{schedule}
      , endPathTask_{endPathTask}
      , principal_{principal}
      , taskGroup_{group}
    {}

    void
    operator()(exception_ptr const ex)
    {
      auto const scheduleID = schedule_->sc_.id();

      TDEBUG_BEGIN_TASK_SI(4, scheduleID);
      if (ex) {
        taskGroup_.may_run(endPathTask_, ex);
        TDEBUG_END_TASK_SI(4, scheduleID)
          << "trigger path processing terminate because of EXCEPTION";
        return;
      }

      try {
        schedule_->process_event_paths_done(principal_);
        taskGroup_.may_run(endPathTask_);
      }
      catch (...) {
        taskGroup_.may_run(endPathTask_, current_exception());
      };

      // Start the endPathTask going.
      TDEBUG_END_TASK_SI(4, scheduleID);
    }

  private:
    TriggerPathsExecutor* const schedule_;
    WaitingTaskPtr const endPathTask_;
    EventPrincipal& principal_;
    GlobalTaskGroup& taskGroup_;
  };

  void
  TriggerPathsExecutor::process_event(WaitingTaskPtr endPathTask,
                                      EventPrincipal& event_principal)
  {
    // We get here as part of the readAndProcessEventTask (schedule
    // head task).
    auto const scheduleID = sc_.id();
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    for (auto const& val : triggerPathsInfo_.workers()) {
      val.second->reset();
    }
    if (results_inserter_) {
      results_inserter_->reset();
    }
    triggerPathsInfo_.pathResults().reset();
    triggerPathsInfo_.incrementTotalEventCount();
    try {
      if (triggerPathsInfo_.paths().empty()) {
        auto pathsDoneTask = make_waiting_task<PathsDoneTask>(
          this, endPathTask, event_principal, taskGroup_);
        taskGroup_.may_run(pathsDoneTask);
        TDEBUG_END_FUNC_SI(4, scheduleID);
        return;
      }
      auto pathsDoneTask = std::make_shared<WaitingTask>(
        PathsDoneTask{this, endPathTask, event_principal, taskGroup_},
        triggerPathsInfo_.paths().size());
      for (auto& path : triggerPathsInfo_.paths()) {
        // Start each path running.  The path will start a spawn chain
        // going to run each worker in the order specified on the
        // path, and when they have all been run, it will call
        // doneWaiting() on the pathsDoneTask, which decrements its
        // reference count, which will eventually cause it to run when
        // every path has finished.
        path->process(pathsDoneTask, event_principal);
      }
      TDEBUG_END_FUNC_SI(4, scheduleID);
    }
    catch (...) {
      taskGroup_.may_run(endPathTask, current_exception());
      TDEBUG_END_FUNC_SI(4, scheduleID) << "because of EXCEPTION";
    }
  }

  void
  TriggerPathsExecutor::process_event_paths_done(EventPrincipal& principal)
  {
    // We come here as part of the pathsDoneTask.
    auto const scheduleID = sc_.id();
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    try {
      if (triggerPathsInfo_.pathResults().accept()) {
        triggerPathsInfo_.incrementPassedEventCount();
      }
      if (results_inserter_) {
        // FIXME: not sure what the trigger bit should be
        auto const& resultsInserterDesc = results_inserter_->description();
        PathContext const pc{sc_,
                             PathContext::art_path(),
                             -1,
                             {resultsInserterDesc.moduleLabel()}};
        ModuleContext const mc{pc, resultsInserterDesc};
        results_inserter_->doWork_event(principal, mc);
      }
    }
    catch (cet::exception& e) {
      auto action = actionTable_.find(e.root_cause());
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action != actions::SkipEvent) {
        TDEBUG_END_FUNC_SI(4, scheduleID);
        throw;
      }
      mf::LogWarning(e.category())
        << "An exception occurred inserting the TriggerResults object:\n"
        << cet::trim_right_copy(e.what(), " \n");
    }
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }
} // namespace art
