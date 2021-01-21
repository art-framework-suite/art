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
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

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

  EndPathExecutor::~EndPathExecutor()
  {
    delete outputWorkersToClose_.load();
    outputWorkersToClose_ = nullptr;
    delete outputWorkersToOpen_.load();
    outputWorkersToOpen_ = nullptr;
    if (auto rsh = subRunRangeSetHandler_.load()) {
      for (auto val : *rsh) {
        delete val;
      }
      delete rsh;
    }
    subRunRangeSetHandler_ = nullptr;
    delete runRangeSetHandler_.load();
    runRangeSetHandler_ = nullptr;
    delete outputWorkers_.load();
    outputWorkers_ = nullptr;
    endPathInfo_ = nullptr;
    actReg_ = nullptr;
    actionTable_ = nullptr;
  }

  EndPathExecutor::EndPathExecutor(ScheduleID const sid,
                                   PathManager& pm,
                                   ActionTable const& actionTable,
                                   ActivityRegistry const& areg,
                                   UpdateOutputCallbacks& outputCallbacks)
    : sc_{sid}
  {
    actionTable_ = &actionTable;
    actReg_ = &areg;
    endPathInfo_ = &pm.endPathInfo(sid);
    runningWorkerCnt_ = 0;
    outputWorkers_ = new vector<OutputWorker*>;
    runRangeSetHandler_ = nullptr;
    subRunRangeSetHandler_ = new PerScheduleContainer<RangeSetHandler*>{1};
    fileStatus_ = OutputFileStatus::Closed;
    outputWorkersToOpen_ = new set<OutputWorker*>;
    outputWorkersToClose_ = new set<OutputWorker*>;
    for (auto const& val : endPathInfo_.load()->workers()) {
      auto w = val.second;
      assert(sid == w->scheduleID());
      auto owp = dynamic_cast<OutputWorker*>(w);
      if (owp != nullptr) {
        outputWorkers_.load()->emplace_back(owp);
      }
    }
    outputWorkersToOpen_.load()->insert(outputWorkers_.load()->cbegin(),
                                        outputWorkers_.load()->cend());
    outputCallbacks.registerCallback(
      [this](auto const& tables) { this->selectProducts(tables); });
  }

  void
  EndPathExecutor::beginJob()
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    Exception error{errors::EndJobFailure};
    // FIXME: There seems to be little value-added by the catch and rethrow
    // here.
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->selectProducts(tables);
    }
  }

  void
  EndPathExecutor::respondToOpenInputFile(FileBlock const& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
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
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return any_of(outputWorkers_.load()->cbegin(),
                  outputWorkers_.load()->cend(),
                  [](auto ow) { return ow->fileIsOpen(); });
  }

  void
  EndPathExecutor::closeAllOutputFiles()
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      actReg_.load()->sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.load()->sPostCloseOutputFile.invoke(
        OutputFileInfo(ow->label(), ow->lastClosedFileName()));
    }
  }

  void
  EndPathExecutor::seedRunRangeSet(RangeSetHandler const& rsh)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    runRangeSetHandler_ = rsh.clone();
  }

  void
  EndPathExecutor::setRunAuxiliaryRangeSetID(RangeSet const& rangeSet)
  {
    for (auto ow : *outputWorkers_.load()) {
      ow->setRunAuxiliaryRangeSetID(rangeSet);
    }
  }

  void
  EndPathExecutor::writeRun(RunPrincipal& rp)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->writeRun(rp);
    }
    if (fileStatus_.load() == OutputFileStatus::Switching) {
      runRangeSetHandler_.load()->rebase();
    }
  }

  void
  EndPathExecutor::seedSubRunRangeSet(RangeSetHandler const& rsh)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& val : *subRunRangeSetHandler_.load()) {
      delete val;
      val = rsh.clone();
    }
  }

  void
  EndPathExecutor::setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      // For RootOutput this enters the possibly split range set into
      // the range set db.
      ow->setSubRunAuxiliaryRangeSetID(rs);
    }
  }

  void
  EndPathExecutor::writeSubRun(SubRunPrincipal& srp)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->writeSubRun(srp);
    }
    if (fileStatus_.load() == OutputFileStatus::Switching) {
      for (auto& rsh : *subRunRangeSetHandler_.load()) {
        rsh->rebase();
      }
    }
  }

  //
  //  MEMBER FUNCTIONS -- Process Non-Event
  //

  void
  EndPathExecutor::process(Transition trans, Principal& principal)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = *label_and_worker.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.reset();
    }
    try {
      if (!endPathInfo_.load()->paths().empty()) {
        endPathInfo_.load()->paths().front()->process(trans, principal);
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
    endPathInfo_.load()->incrementPassedEventCount();
  }

  // Note: We come here as part of the endPath task, our
  // parent task is the eventLoop task.
  void
  EndPathExecutor::process_event(EventPrincipal& ep)
  {
    auto const sid = sc_.id();
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    TDEBUG_BEGIN_FUNC_SI(4, sid);
    if (runningWorkerCnt_.load() != 0) {
      cerr << "Aborting! runningWorkerCnt_.load() != 0: "
           << runningWorkerCnt_.load() << "\n";
      abort();
    }
    ++runningWorkerCnt_;
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->reset();
    }
    endPathInfo_.load()->incrementTotalEventCount();
    try {
      if (!endPathInfo_.load()->paths().empty()) {
        endPathInfo_.load()->paths().front()->process_event_for_endpath(ep);
      }
    }
    catch (cet::exception& ex) {
      // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
      // FailPath
      auto const action{actionTable_.load()->find(ex.root_cause())};
      if (action != actions::IgnoreCompletely) {
        // Possible actions: Rethrow, SkipEvent, FailModule, FailPath
        TDEBUG_END_FUNC_SI(4, sid) << "because of EXCEPTION";
        if (runningWorkerCnt_.load() != 1) {
          abort();
        }
        --runningWorkerCnt_;
        throw Exception(errors::EventProcessorFailure, "EndPathExecutor:")
          << "an exception occurred during current event processing\n"
          << ex;
      }
      // Possible actions: IgnoreCompletely
      mf::LogWarning(ex.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(ex.what(), " \n");
      // WARNING: Processing continues below!!!
      // WARNING: We can only get here if an exception in end path
      // processing is being ignored for the current event because
      // the action is actions::IgnoreCompletely.
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      TDEBUG_END_FUNC_SI(4, sid) << "because of EXCEPTION";
      if (runningWorkerCnt_.load() != 1) {
        abort();
      }
      --runningWorkerCnt_;
      throw;
    }
    endPathInfo_.load()->incrementPassedEventCount();
    if (runningWorkerCnt_.load() != 1) {
      abort();
    }
    --runningWorkerCnt_;
    TDEBUG_END_FUNC_SI(4, sid);
  }

  void
  EndPathExecutor::writeEvent(EventPrincipal& ep)
  {
    // We don't worry about providing the sorted list of module names
    // for the end_path right now.  If users decide it is necessary to
    // know what they are, then we can provide them.
    PathContext const pc{sc_, PathContext::end_path(), 0, {}};
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ModuleContext const mc{pc, ow->description()};
      actReg_.load()->sPreWriteEvent.invoke(mc);
      ow->writeEvent(ep);
      actReg_.load()->sPostWriteEvent.invoke(mc);
    }
    auto const& eid = ep.eventID();
    bool const lastInSubRun{ep.isLastInSubRun()};
    TDEBUG_FUNC_SI(5, sc_.id())
      << "eid: " << eid.run() << ", " << eid.subRun() << ", " << eid.event();
    runRangeSetHandler_.load()->update(eid, lastInSubRun);
    subRunRangeSetHandler_.load()
      ->at(ScheduleID::first())
      ->update(eid, lastInSubRun);
  }

  bool
  EndPathExecutor::outputsToClose() const
  {
    return !outputWorkersToClose_.load()->empty();
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
    for (auto ow : *outputWorkersToClose_) {
      // Skip files that are already closed due to other end-path
      // executors already closing them.
      if (!ow->fileIsOpen()) {
        continue;
      }
      actReg_.load()->sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.load()->sPostCloseOutputFile.invoke(
        OutputFileInfo{ow->label(), ow->lastClosedFileName()});
    }
    *outputWorkersToOpen_.load() = move(*outputWorkersToClose_.load());
  }

  bool
  EndPathExecutor::outputsToOpen() const
  {
    return !outputWorkersToOpen_.load()->empty();
  }

  void
  EndPathExecutor::openSomeOutputFiles(FileBlock const& fb)
  {
    for (auto ow : *outputWorkersToOpen_.load()) {
      if (!ow->openFile(fb)) {
        continue;
      }
      actReg_.load()->sPostOpenOutputFile.invoke(ow->label());
    }
    setOutputFileStatus(OutputFileStatus::Open);
    outputWorkersToOpen_.load()->clear();
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
    for (auto ow : *outputWorkers_.load()) {
      ow->setFileStatus(ofs);
    }
    fileStatus_ = ofs;
  }

  void
  EndPathExecutor::recordOutputClosureRequests(Granularity const atBoundary)
  {
    for (auto ow : *outputWorkers_.load()) {
      if (atBoundary < ow->fileGranularity()) {
        // The boundary we are checking at is finer than the checks
        // the output worker needs, nothing to do.
        continue;
      }
      auto wants_to_close = ow->requestsToCloseFile();
      if (wants_to_close) {
        outputWorkersToClose_.load()->insert(ow);
      }
    }
  }

  void
  EndPathExecutor::incrementInputFileNumber()
  {
    for (auto ow : *outputWorkers_.load()) {
      ow->incrementInputFileNumber();
    }
  }

  bool
  EndPathExecutor::allAtLimit() const
  {
    if (outputWorkers_.load()->empty()) {
      return false;
    }
    bool all_at_limit = true;
    for (auto w : *outputWorkers_.load()) {
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
