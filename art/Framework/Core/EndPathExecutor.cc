#include "art/Framework/Core/EndPathExecutor.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/OutputFileInfo.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/trim.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "tbb/task.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {

  EndPathExecutor::EndPathExecutor(ScheduleID const sid,
                                   PathManager& pm,
                                   ActionTable const& actionTable,
                                   ActivityRegistry const& areg,
                                   UpdateOutputCallbacks& outputCallbacks)
    : sc_{sid}
    , actionTable_{actionTable}
    , actReg_{areg}
    , endPathInfo_{pm.endPathInfo(sid)}
  {
    for (auto const& val : endPathInfo_.workers()) {
      auto w = val.second;
      assert(sid == w->scheduleID());
      auto owp = dynamic_cast<OutputWorker*>(w);
      if (owp != nullptr) {
        outputWorkers_.emplace_back(owp);
      }
    }
    outputWorkersToOpen_.insert(outputWorkers_.cbegin(), outputWorkers_.cend());
    outputCallbacks.registerCallback(
      [this](auto const& tables) { this->selectProducts(tables); });
  }

  void
  EndPathExecutor::beginJob()
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.beginJob();
    }
  }

  void
  EndPathExecutor::endJob()
  {
    Exception error{errors::EndJobFailure};
    // FIXME: There seems to be little value-added by the catch and rethrow
    // here.
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
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
  }

  void
  EndPathExecutor::selectProducts(ProductTables const& tables)
  {
    for (auto ow : outputWorkers_) {
      ow->selectProducts(tables);
    }
  }

  void
  EndPathExecutor::respondToOpenInputFile(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenOutputFiles(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseOutputFiles(fb);
    }
  }

  bool
  EndPathExecutor::someOutputsOpen() const
  {
    return any_of(outputWorkers_.cbegin(), outputWorkers_.cend(), [](auto ow) {
      return ow->fileIsOpen();
    });
  }

  void
  EndPathExecutor::closeAllOutputFiles()
  {
    for (auto ow : outputWorkers_) {
      actReg_.sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.sPostCloseOutputFile.invoke(
        OutputFileInfo(ow->label(), ow->lastClosedFileName()));
    }
  }

  void
  EndPathExecutor::seedRunRangeSet(RangeSetHandler const& rsh)
  {
    runRangeSetHandler_.reset(rsh.clone());
  }

  void
  EndPathExecutor::setRunAuxiliaryRangeSetID(RangeSet const& rangeSet)
  {
    for (auto ow : outputWorkers_) {
      ow->setRunAuxiliaryRangeSetID(rangeSet);
    }
  }

  void
  EndPathExecutor::writeRun(RunPrincipal& rp)
  {
    for (auto ow : outputWorkers_) {
      ow->writeRun(rp);
    }
    if (fileStatus_.load() == OutputFileStatus::Switching) {
      runRangeSetHandler_->rebase();
    }
  }

  void
  EndPathExecutor::seedSubRunRangeSet(RangeSetHandler const& rsh)
  {
    subRunRangeSetHandler_.reset(rsh.clone());
  }

  void
  EndPathExecutor::setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    for (auto ow : outputWorkers_) {
      // For RootOutput this enters the possibly split range set into
      // the range set db.
      ow->setSubRunAuxiliaryRangeSetID(rs);
    }
  }

  void
  EndPathExecutor::writeSubRun(SubRunPrincipal& srp)
  {
    for (auto ow : outputWorkers_) {
      ow->writeSubRun(srp);
    }
    if (fileStatus_.load() == OutputFileStatus::Switching) {
      subRunRangeSetHandler_->rebase();
    }
  }

  //
  //  MEMBER FUNCTIONS -- Process Non-Event
  //

  void
  EndPathExecutor::process(Transition trans, Principal& principal)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.reset();
    }
    try {
      if (!endPathInfo_.paths().empty()) {
        endPathInfo_.paths().front()->process(trans, principal);
      }
    }
    catch (cet::exception& ex) {
      throw Exception(errors::EventProcessorFailure, "EndPathExecutor:")
        << "an exception occurred during current event processing\n"
        << ex;
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      throw;
    }
    endPathInfo_.incrementPassedEventCount();
  }

  class EndPathExecutor::PathsDoneTask {
  public:
    PathsDoneTask(EndPathExecutor* const endPathExec,
                  tbb::task* const finalizeEventTask)
      : endPathExec_{endPathExec}, finalizeEventTask_{finalizeEventTask}
    {}

    void
    operator()(exception_ptr const ex)
    {
      WaitingTaskHolder wth(finalizeEventTask_);
      auto const scheduleID = endPathExec_->sc_.id();

      // Note: When we start our parent task is the eventLoop task.
      TDEBUG_BEGIN_TASK_SI(4, scheduleID);

      if (ex) {
        try {
          rethrow_exception(ex);
        }
        catch (cet::exception& e) {
          Exception tmp(errors::EventProcessorFailure, "EndPathExecutor:");
          tmp << "an exception occurred during current event processing\n" << e;
          wth.doneWaiting(make_exception_ptr(tmp));
          TDEBUG_END_TASK_SI(4, scheduleID)
            << "end path processing terminate because of EXCEPTION";
          return;
        }
        catch (...) {
          wth.doneWaiting(current_exception());
          TDEBUG_END_TASK_SI(4, scheduleID)
            << "end path processing terminate because of EXCEPTION";
          return;
        }
      }

      endPathExec_->endPathInfo_.incrementPassedEventCount();

      // Start the finalizeEventTask going.
      wth.doneWaiting();
      TDEBUG_END_TASK_SI(4, scheduleID);
    }

  private:
    EndPathExecutor* const endPathExec_;
    tbb::task* const finalizeEventTask_;
  };

  // Note: We come here as part of the endPath task, our
  // parent task is the eventLoop task.
  void
  EndPathExecutor::process_event(tbb::task* finalizeEventTask,
                                 EventPrincipal& ep)
  {
    auto const sid = sc_.id();
    TDEBUG_BEGIN_FUNC_SI(4, sid);
    for (auto& label_and_worker : endPathInfo_.workers()) {
      label_and_worker.second->reset();
    }
    endPathInfo_.incrementTotalEventCount();
    try {
      auto pathsDoneTask = make_waiting_task(
        tbb::task::allocate_root(), PathsDoneTask{this, finalizeEventTask});
      // Note: We create the holder here to increment the ref count on
      // the pathsDoneTask so that if a path errors quickly and
      // decrements the ref count (using doneWaiting) the task will
      // not run until we have actually started all the tasks.  Note:
      // This is critically dependent on the path incrementing the ref
      // count the first thing it does (by putting the task into a
      // WaitingTaskList).
      WaitingTaskHolder wth(pathsDoneTask);
      if (!endPathInfo_.paths().empty()) {
        endPathInfo_.paths().front()->process(pathsDoneTask, ep);
      }
    }
    catch (...) {
      WaitingTaskHolder wth(finalizeEventTask);
      wth.doneWaiting(current_exception());
    }
    TDEBUG_END_FUNC_SI(4, sid);
  }

  void
  EndPathExecutor::writeEvent(EventPrincipal& ep)
  {
    // We don't worry about providing the sorted list of module names
    // for the end_path right now.  If users decide it is necessary to
    // know what they are, then we can provide them.
    PathContext const pc{sc_, PathContext::end_path(), 0, {}};
    for (auto ow : outputWorkers_) {
      ModuleContext const mc{pc, ow->description()};
      actReg_.sPreWriteEvent.invoke(mc);
      ow->writeEvent(ep);
      actReg_.sPostWriteEvent.invoke(mc);
    }
    auto const& eid = ep.eventID();
    bool const lastInSubRun{ep.isLastInSubRun()};
    TDEBUG_FUNC_SI(5, sc_.id())
      << "eid: " << eid.run() << ", " << eid.subRun() << ", " << eid.event();
    runRangeSetHandler_->update(eid, lastInSubRun);
    subRunRangeSetHandler_->update(eid, lastInSubRun);
  }

  bool
  EndPathExecutor::outputsToClose() const
  {
    return !outputWorkersToClose_.empty();
  }

  // MT note: This is where we need to get all the schedules
  //          synchronized, and then have all schedules do the file
  //          close, and then the file open, then the schedules can
  //          proceed.  A nasty complication is that a great deal of
  //          time can go by between the file close and the file open
  //          because artdaq may pause the run inbetween, and wants to
  //          have all output files closed while the run is paused.
  //          They probably want the input file closed too.
  void
  EndPathExecutor::closeSomeOutputFiles()
  {
    setOutputFileStatus(OutputFileStatus::Switching);
    for (auto ow : outputWorkersToClose_) {
      // Skip files that are already closed due to other end-path
      // executors already closing them.
      if (!ow->fileIsOpen()) {
        continue;
      }
      actReg_.sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.sPostCloseOutputFile.invoke(
        OutputFileInfo{ow->label(), ow->lastClosedFileName()});
    }
    outputWorkersToOpen_ = move(outputWorkersToClose_);
  }

  bool
  EndPathExecutor::outputsToOpen() const
  {
    return !outputWorkersToOpen_.empty();
  }

  void
  EndPathExecutor::openSomeOutputFiles(FileBlock const& fb)
  {
    for (auto ow : outputWorkersToOpen_) {
      if (!ow->openFile(fb)) {
        continue;
      }
      actReg_.sPostOpenOutputFile.invoke(ow->label());
    }
    setOutputFileStatus(OutputFileStatus::Open);
    outputWorkersToOpen_.clear();
  }

  // Note: When we are passed OutputFileStatus::Switching, we must close
  //       the file and call openSomeOutputFiles which changes it back
  //       to OutputFileStatus::Open.
  //       A side effect of switching status is the run/subrun writes
  //       are not counted in the overall counting by RootOutputClosingCriteria
  //       while the switch is active (this avoids counting the extra subRun and
  //       Run that we are forced to write to finish out the file we are
  //       closing, which keeps the ongoing count for closing based on SubRun
  //       and Run counts meaningful).  However, the extra ones are still
  //       counted by the tree entry counters.
  void
  EndPathExecutor::setOutputFileStatus(OutputFileStatus const ofs)
  {
    for (auto ow : outputWorkers_) {
      ow->setFileStatus(ofs);
    }
    fileStatus_ = ofs;
  }

  void
  EndPathExecutor::recordOutputClosureRequests(Granularity const atBoundary)
  {
    for (auto ow : outputWorkers_) {
      if (atBoundary < ow->fileGranularity()) {
        // The boundary we are checking at is finer than the checks
        // the output worker needs, nothing to do.
        continue;
      }
      auto wants_to_close = ow->requestsToCloseFile();
      if (wants_to_close) {
        outputWorkersToClose_.insert(ow);
      }
    }
  }

  void
  EndPathExecutor::incrementInputFileNumber()
  {
    for (auto ow : outputWorkers_) {
      ow->incrementInputFileNumber();
    }
  }

  bool
  EndPathExecutor::allAtLimit() const
  {
    if (outputWorkers_.empty()) {
      return false;
    }
    bool all_at_limit = true;
    for (auto w : outputWorkers_) {
      if (!w->limitReached()) {
        all_at_limit = false;
        break;
      }
    }
    if (all_at_limit) {
      mf::LogInfo("SuccessfulTermination")
        << "The job is terminating successfully because each output module\n"
        << "has reached its configured limit.\n";
    }
    return all_at_limit;
  }

} // namespace art
