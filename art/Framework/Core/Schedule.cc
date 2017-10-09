#include "art/Framework/Core/Schedule.h"
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
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/Transition.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "canvas/Utilities/DebugMacros.h"
#include "cetlib/exempt_ptr.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"

#include <algorithm>
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

  Schedule::Schedule(int stream,
                     PathManager& pm,
                     string const& processName,
                     ParameterSet const& proc_pset,
                     UpdateOutputCallbacks& outputCallbacks,
                     ProductDescriptions& productsToProduce,
                     ActionTable& actions,
                     ActivityRegistry& actReg)
    : stream_{stream}
    , process_pset_{proc_pset}
    , outputCallbacks_{outputCallbacks}
    , actionTable_{actions}
    , actReg_{actReg}
    , processName_{processName}
    , triggerPathsInfo_{pm.triggerPathsInfo(stream)}
  {
    TDEBUG(5) << "Schedule ctor: 0x" << hex << ((unsigned long)this) << dec
              << " (" << stream << ")\n";
    if (!triggerPathsInfo_.paths().empty()) {
      if ((results_inserter_ = pm.triggerResultsInserter(stream)) == nullptr) {
        // Make the trigger results inserter.
        ServiceHandle<TriggerNamesService const> tns;
        auto const& trig_pset = tns->getTriggerPSet();
        WorkerParams const wp{process_pset_,
                              trig_pset,
                              outputCallbacks_,
                              productsToProduce,
                              actReg_,
                              actionTable_,
                              processName_,
                              ModuleThreadingType::STREAM,
                              stream};
        ModuleDescription md{trig_pset.id(),
                             "TriggerResultInserter",
                             "TriggerResults",
                             static_cast<int>(ModuleThreadingType::STREAM),
                             ProcessConfiguration{processName_,
                                                  process_pset_.id(),
                                                  getReleaseVersion()}};
        actReg_.sPreModuleConstruction.invoke(md);
        EDProducer* producer = new TriggerResultInserter(
          trig_pset, stream, triggerPathsInfo_.pathResults());
        producer->setModuleDescription(md);
        producer->setStreamIndex(stream);
        pm.setTriggerResultsInserter(
          stream, make_unique<WorkerT<EDProducer>>(producer, md, wp));
        results_inserter_ = pm.triggerResultsInserter(stream);
        actReg_.sPostModuleConstruction.invoke(md);
      }
    }
  }

  void
  Schedule::beginJob()
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.beginJob();
    }
    if (results_inserter_) {
      results_inserter_->beginJob();
    }
  }

  void
  Schedule::endJob()
  {
    Exception error(errors::EndJobFailure);
    for (auto& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        w.endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in Schedule::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in Schedule::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in Schedule::endJob\n";
        throw error;
      }
    }
    if (results_inserter_) {
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        results_inserter_->endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in Schedule::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in Schedule::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in Schedule::endJob\n";
        throw error;
      }
    }
  }

  void
  Schedule::respondToOpenInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.respondToOpenInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenInputFile(fb);
    }
  }

  void
  Schedule::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.respondToCloseInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseInputFile(fb);
    }
  }

  void
  Schedule::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.respondToOpenOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenOutputFiles(fb);
    }
  }

  void
  Schedule::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.respondToCloseOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseOutputFiles(fb);
    }
  }

  void
  Schedule::process(Transition trans, Principal& principal)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      val.second->reset(0);
    }
    for (auto const& path : triggerPathsInfo_.paths()) {
      path->process(trans, principal);
    }
  }

  // Note: We get here as part of the readAndProcessEvent task.
  // Our parent task is the nullptr, and the parent task of the
  // endPathTask is the eventLoopTask.
  void
  Schedule::process_event(WaitingTask* endPathTask,
                          EventPrincipal& principal,
                          int si)
  {
    TDEBUG(4) << "-----> Begin Schedule::process_event (" << si << ") ...\n";
    auto eventLoopTask = endPathTask->parent();
    for (auto const& val : triggerPathsInfo_.workers()) {
      auto& w = *val.second;
      w.reset(si);
    }
    if (results_inserter_) {
      results_inserter_->reset(si);
    }
    triggerPathsInfo_.pathResults().reset();
    triggerPathsInfo_.incrementTotalEventCount();
    auto pathsDoneFunctor = [this, endPathTask, &principal, si](
                              exception_ptr const* ex) mutable {
      // Note: When we start our parent task is the eventLoop task.
      TDEBUG(4) << "=====> Begin pathsDoneTask (" << si << ") ...\n";
      if (ex != nullptr) {
        try {
          rethrow_exception(*ex);
        }
        catch (cet::exception& e) {
          auto action = actionTable_.find(e.root_cause());
          assert(action != actions::IgnoreCompletely);
          assert(action != actions::FailPath);
          assert(action != actions::FailModule);
          if (action != actions::SkipEvent) {
            WaitingTaskHolder wth(endPathTask);
            wth.doneWaiting(current_exception());
            // And end this task which does not terminate event processing.
            tbb::task::self().set_parent(nullptr);
            TDEBUG(4) << "=====> End   pathsDoneTask (" << si
                      << ") ... trigger path processing terminate because of "
                         "exception\n";
            return;
          }
          // FIXME: We should not ever be able to get here because the only
          // exceptions
          // FIXME: passed to the pathsDone task should be ones that terminated
          // the path.
          mf::LogWarning(e.category()) << "an exception occurred and all paths "
                                          "for the event are being skipped: \n"
                                       << cet::trim_right_copy(e.what(), " \n");
        }
        // FIXME: We should not ever be able to get here because the only
        // exceptions
        // FIXME: passed to the pathsDone task should be ones that terminated
        // the path. Transfer the thrown exception to the endPath task and start
        // it running.
        WaitingTaskHolder wth(endPathTask);
        wth.doneWaiting(*ex);
        // And end this task without terminating
        // event processing.
        tbb::task::self().set_parent(nullptr);
        TDEBUG(4)
          << "=====> End   pathsDoneTask (" << si
          << ") ... trigger path processing terminate because of exception\n";
        return;
      }
      process_event_pathsDone(endPathTask, principal, si);
      // And end this task, which does not terminate
      // event processing because our parent is the
      // nullptr.
      TDEBUG(4) << "=====> End   pathsDoneTask (" << si << ") ...\n";
      return;
    };
    auto pathsDoneTask =
      make_waiting_task(tbb::task::allocate_root(), pathsDoneFunctor);
    pathsDoneTask->set_parent(eventLoopTask);
    try {
      // Note: We create the holder here to increment the ref count
      // on the pathsDoneTask so that if a path errors quickly and
      // decrements the ref count (using doneWaiting) the task will
      // not run until we have actually started all the tasks.
      // Note: This is critically dependent on the path incrementing
      // the ref count the first thing it does (by putting the task
      // into a WaitingTaskList).
      WaitingTaskHolder wth(pathsDoneTask);
      for (auto& path : triggerPathsInfo_.paths()) {
        // Start each path running.  The path will start a spawn
        // chain going to run each worker in the order specified
        // on the path, and when they have all been run, it will
        // call doneWaiting() on the pathsDoneTask, which decrements
        // its reference count, which will eventually cause it to
        // run when every path has finished.
        path->process_event(pathsDoneTask, principal, si);
      }
      // And end this task which does not terminate event processing
      // because our parent is the nullptr.
      TDEBUG(4) << "-----> End   Schedule::process_event (" << si << ") ...\n";
      return;
    }
    catch (cet::exception& e) {
      auto action = actionTable_.find(e.root_cause());
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action != actions::SkipEvent) {
        WaitingTaskHolder wth(endPathTask);
        wth.doneWaiting(current_exception());
        // And end this task which does not terminate event processing
        // because our parent is the nullptr.
        TDEBUG(4) << "-----> End   Schedule::process_event (" << si
                  << ") ... because of exception\n";
        return;
      }
      mf::LogWarning(e.category()) << "an exception occurred and all paths for "
                                      "the event are being skipped: \n"
                                   << cet::trim_right_copy(e.what(), " \n");
    }
    WaitingTaskHolder wth(endPathTask);
    wth.doneWaiting(exception_ptr{});
    // And end this task which does not terminate event processing
    // because our parent is the nullptr.
    TDEBUG(4) << "-----> End   Schedule::process_event (" << si << ") ...\n";
  }

  // Note: We come here as part of the pathsDone task.  Our parent
  // is the eventLoopTask.
  void
  Schedule::process_event_pathsDone(WaitingTask* endPathTask,
                                    EventPrincipal& principal,
                                    int si)
  {
    TDEBUG(4) << "-----> Begin Schedule::process_event_pathsDone (" << si
              << ") ...\n";
    try {
      if (triggerPathsInfo_.pathResults().accept()) {
        triggerPathsInfo_.incrementPassedEventCount();
      }
      if (results_inserter_) {
        string const name{"TriggerResultsInserter"};
        CurrentProcessingContext cpc{si, &name, 0, false};
        results_inserter_->doWork_event(principal, si, &cpc);
      }
    }
    catch (cet::exception& e) {
      auto action = actionTable_.find(e.root_cause());
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action != actions::SkipEvent) {
        WaitingTaskHolder wth(endPathTask);
        wth.doneWaiting(current_exception());
        // And end this task without terminating
        // event processing.
        tbb::task::self().set_parent(nullptr);
        TDEBUG(4) << "-----> End   Schedule::process_event_pathsDone (" << si
                  << ") ...\n";
        return;
      }
      mf::LogWarning(e.category()) << "an exception occurred and all paths for "
                                      "the event are being skipped:\n"
                                   << cet::trim_right_copy(e.what(), " \n");
    }
    WaitingTaskHolder wth(endPathTask);
    wth.doneWaiting(exception_ptr{});
    // And end this task without terminating
    // event processing.
    tbb::task::self().set_parent(nullptr);
    TDEBUG(4) << "-----> End   Schedule::process_event_pathsDone (" << si
              << ") ...\n";
  }

} // namespace art
