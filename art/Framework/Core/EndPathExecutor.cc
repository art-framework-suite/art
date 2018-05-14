#include "art/Framework/Core/EndPathExecutor.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
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
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/trim.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
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
    for (auto val : *subRunRangeSetHandler_.load()) {
      delete val;
    }
    delete subRunRangeSetHandler_.load();
    subRunRangeSetHandler_ = nullptr;
    for (auto val : *runRangeSetHandler_.load()) {
      delete val;
    }
    delete runRangeSetHandler_.load();
    runRangeSetHandler_ = nullptr;
    delete outputWorkers_.load();
    outputWorkers_ = nullptr;
    delete serialTaskQueue_.load();
    serialTaskQueue_ = nullptr;
    endPathInfo_ = nullptr;
    actReg_ = nullptr;
    actionTable_ = nullptr;
  }

  EndPathExecutor::EndPathExecutor(PathManager& pm,
                                   ActionTable const& actionTable,
                                   ActivityRegistry const& areg,
                                   UpdateOutputCallbacks& outputCallbacks)
  {
    actionTable_ = &actionTable;
    actReg_ = &areg;
    endPathInfo_ = &pm.endPathInfo();
    serialTaskQueue_ = new SerialTaskQueue;
    runningWorkerCnt_ = 0;
    outputWorkers_ = new vector<OutputWorker*>;
    runRangeSetHandler_ = new PerScheduleContainer<RangeSetHandler*>;
    runRangeSetHandler_.load()->expand_to_num_schedules();
    subRunRangeSetHandler_ = new PerScheduleContainer<RangeSetHandler*>;
    subRunRangeSetHandler_.load()->expand_to_num_schedules();
    fileStatus_ = OutputFileStatus::Closed;
    outputWorkersToOpen_ = new set<OutputWorker*>;
    outputWorkersToClose_ = new set<OutputWorker*>;
    for (auto const& val : endPathInfo_.load()->workers()) {
      auto w = val.second;
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
  EndPathExecutor::check()
  {
    return;
    int idx = 0;
    for (auto& val : *subRunRangeSetHandler_.load()) {
      auto rsh = dynamic_cast<ClosedRangeSetHandler*>(val);
      if (rsh == nullptr) {
        ++idx;
        continue;
      }
      if (!rsh->eventInfo().id().isValid()) {
        cerr << "\n";
        ostringstream buf;
        buf << "EndPathExecutor::check: idx: " << idx << "\n";
        cerr << buf.str();
        cerr << "\n";
      }
      ++idx;
    }
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End Job
  //

  void
  EndPathExecutor::beginJob()
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->beginJob();
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
      auto& w = label_and_worker.second;
      try {
        w->endJob();
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

  //
  //  MEMBER FUNCTIONS -- Input File Open/Close.
  //

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
      auto& w = label_and_worker.second;
      w->respondToOpenInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseInputFile(FileBlock const& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->respondToCloseInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToOpenOutputFiles(FileBlock const& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->respondToOpenOutputFiles(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseOutputFiles(FileBlock const& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->respondToCloseOutputFiles(fb);
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
  EndPathExecutor::openAllOutputFiles(FileBlock& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->openFile(fb);
      actReg_.load()->sPostOpenOutputFile.invoke(ow->label());
    }
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End Run
  //

  void
  EndPathExecutor::seedRunRangeSet(unique_ptr<RangeSetHandler> rsh)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& val : *runRangeSetHandler_.load()) {
      delete val;
      val = rsh->clone();
    }
  }

  void
  EndPathExecutor::setAuxiliaryRangeSetID(RunPrincipal& rp)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (runRangeSetHandler_.load()->at(ScheduleID::first())->type() ==
        RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the schedules have seen.
      RangeSet mergedSeenRanges;
      for (auto& uptr_rsh : *runRangeSetHandler_.load()) {
        mergedSeenRanges.merge(uptr_rsh->seenRanges());
      }
      rp.updateSeenRanges(mergedSeenRanges);
      for (auto ow : *outputWorkers_.load()) {
        ow->setRunAuxiliaryRangeSetID(mergedSeenRanges);
      }
      return;
    }
    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges, use the first one.
    // handler with the largest event number, that will be the
    // one which we will use as the file switch boundary.  Note
    // that is may not match the exactly the schedule that triggered
    // the switch.  Do we need to fix this?
    unique_ptr<RangeSetHandler> rshAtSwitch{
      runRangeSetHandler_.load()->at(ScheduleID::first())->clone()};
    if (fileStatus_.load() != OutputFileStatus::Switching) {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    rp.updateSeenRanges(rshAtSwitch->seenRanges());
    for (auto ow : *outputWorkers_.load()) {
      ow->setRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
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
      for (auto& rsh : *runRangeSetHandler_.load()) {
        rsh->rebase();
      }
    }
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End SubRun
  //

  // Called by EventProcessor::readSubRun() just after
  // reading the subrun.
  void
  EndPathExecutor::seedSubRunRangeSet(unique_ptr<RangeSetHandler> rsh)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto& val : *subRunRangeSetHandler_.load()) {
      delete val;
      val = rsh->clone();
    }
  }

  // Called by EventProcessor::finalize<Level::SubRun>()
  // just before endSubRun() and writeSubRun().
  void
  EndPathExecutor::setAuxiliaryRangeSetID(SubRunPrincipal& srp)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (subRunRangeSetHandler_.load()->at(ScheduleID::first())->type() ==
        RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the schedules have seen.
      RangeSet mergedRS;
      for (auto& rsh : *subRunRangeSetHandler_.load()) {
        mergedRS.merge(rsh->seenRanges());
      }
      srp.updateSeenRanges(mergedRS);
      for (auto ow : *outputWorkers_.load()) {
        // For RootOutput this enters the possibly split
        // range set into the range set db.
        ow->setSubRunAuxiliaryRangeSetID(mergedRS);
      }
      return;
    }
    // Ranges are split/flushed only for a RangeSetHandler whose dynamic
    // type is 'ClosedRangeSetHandler'.
    //
    // Consider the following range-sets
    //
    //  SubRun RangeSet:
    //
    //    { Run 1 : SubRun 1 : Events [1,7) }  <-- Current
    //
    //  Run RangeSet:
    //
    //    { Run 1 : SubRun 0 : Events [5,11)
    //              SubRun 1 : Events [1,7)    <-- Current
    //              SubRun 1 : Events [9,15) }
    //
    // For a range split just before SubRun 1, Event 6, the
    // range sets should become:
    //
    //  SubRun RangeSet:
    //
    //    { Run 1 : SubRun 1 : Events [1,6)
    //              SubRun 1 : Events [6,7) } <-- Updated
    //
    //  Run RangeSet:
    //
    //    { Run 1 : SubRun 0 : Events [5,11)
    //              SubRun 1 : Events [1,6)
    //              SubRun 1 : Events [6,7)   <-- Updated
    //              SubRun 1 : Events [9,15) }
    //
    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges.  Find the closed range set
    // handler with the largest event number, that will be the
    // one which we will use as the file switch boundary.  Note
    // that is may not match the exactly the schedule that triggered
    // the switch.  Do we need to fix this?
    //
    // If we do not find any handlers with valid event info then
    // we use the first one, which is just fine.  This happens
    // for example when we are dropping all events.
    //
    unsigned largestEvent = 1U;
    ScheduleID idxOfMax{ScheduleID::first()};
    ScheduleID idx{ScheduleID::first()};
    for (auto& val : *subRunRangeSetHandler_.load()) {
      auto rsh = dynamic_cast<ClosedRangeSetHandler*>(val);
      // Make sure the event number is a valid event number
      // before using it. It can be invalid in the handler if
      // we have not yet read an event, which happens with empty
      // subruns and when we are dropping all events.
      if (rsh->eventInfo().id().isValid() && !rsh->eventInfo().id().isFlush()) {
        if (rsh->eventInfo().id().event() > largestEvent) {
          largestEvent = rsh->eventInfo().id().event();
          idxOfMax = idx;
        }
      }
      idx = idx.next();
    }
    unique_ptr<RangeSetHandler> rshAtSwitch{
      subRunRangeSetHandler_.load()->at(idxOfMax)->clone()};
    if (fileStatus_.load() == OutputFileStatus::Switching) {
      rshAtSwitch->maybeSplitRange();
      unique_ptr<RangeSetHandler> runRSHAtSwitch{
        runRangeSetHandler_.load()->at(idxOfMax)->clone()};
      runRSHAtSwitch->maybeSplitRange();
      for (auto& rsh : *runRangeSetHandler_.load()) {
        rsh = runRSHAtSwitch->clone();
      }
    } else {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    for (auto& val : *subRunRangeSetHandler_.load()) {
      delete val;
      val = rshAtSwitch->clone();
    }
    srp.updateSeenRanges(rshAtSwitch->seenRanges());
    for (auto ow : *outputWorkers_.load()) {
      // For RootOutput this enters the possibly split
      // range set into the range set db.
      ow->setSubRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
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
      auto& w = label_and_worker.second;
      w->reset(ScheduleID::first());
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

  //
  //  MEMBER FUNCTIONS -- Process Event
  //

  // Note: We come here as part of the endPath task, our
  // parent task is the eventLoop task.
  void
  EndPathExecutor::process_event(EventPrincipal& ep, ScheduleID const sid)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    TDEBUG_BEGIN_FUNC_SI(4, "EndPathExecutor::process_event", sid);
    if (runningWorkerCnt_.load() != 0) {
      cerr << "Aborting! runningWorkerCnt_.load() != 0: "
           << runningWorkerCnt_.load() << "\n";
      abort();
    }
    ++runningWorkerCnt_;
    for (auto& label_and_worker : endPathInfo_.load()->workers()) {
      auto& w = label_and_worker.second;
      w->reset(sid);
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
        TDEBUG_END_FUNC_SI_ERR(
          4, "EndPathExecutor::process_event", sid, "because of EXCEPTION");
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
      TDEBUG_END_FUNC_SI_ERR(
        4, "EndPathExecutor::process_event", sid, "because of EXCEPTION");
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
    TDEBUG_END_FUNC_SI(4, "EndPathExecutor::process_event", sid);
  }

  void
  EndPathExecutor::writeEvent(ScheduleID const sid, EventPrincipal& ep)
  {
    ScheduleContext const sc{sid};
    PathContext const pc{sc, "end_path", true};
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      auto const& md = ow->description();
      ModuleContext const mc{pc, md};
      actReg_.load()->sPreWriteEvent.invoke(mc);
      ow->writeEvent(ep);
      actReg_.load()->sPostWriteEvent.invoke(mc);
    }
    auto const& eid = ep.eventID();
    bool const lastInSubRun{ep.isLastInSubRun()};
    {
      ostringstream msg;
      msg << "eid: ";
      msg << eid.run();
      msg << ", ";
      msg << eid.subRun();
      msg << ", ";
      msg << eid.event();
      TDEBUG_FUNC_SI_MSG(5, "EndPathExecutor::writeEvent", sid, msg.str());
    }
    runRangeSetHandler_.load()->at(sid)->update(eid, lastInSubRun);
    subRunRangeSetHandler_.load()->at(sid)->update(eid, lastInSubRun);
  }

  //
  //  MEMBER FUNCTIONS -- Output File Switching API
  //
  //  See also:
  //
  //    respondToOpenOutputFiles(FileBlock const& fb);
  //    respondToCloseOutputFiles(FileBlock const& fb);
  //

  // Called by EventProcessor::closeSomeOutputFiles(), which is called when
  // output file switching is happening. Note: This is really returns
  // !outputWorkersToClose_.load()->empty()
  bool
  EndPathExecutor::outputsToClose() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    return !outputWorkersToClose_.load()->empty();
  }

  // Called by EventProcessor::closeSomeOutputFiles(), which is called when
  // output file switching is happening.
  //
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
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    auto invoke_sPreCloseOutputFile = [this](auto ow) {
      actReg_.load()->sPreCloseOutputFile.invoke(ow->label());
    };
    auto closeFile = [](auto ow) { ow->closeFile(); };
    auto invoke_sPostCloseOutputFile = [this](auto ow) {
      actReg_.load()->sPostCloseOutputFile.invoke(
        OutputFileInfo{ow->label(), ow->lastClosedFileName()});
    };
    setOutputFileStatus(OutputFileStatus::Switching);
    cet::for_all(*outputWorkersToClose_.load(), invoke_sPreCloseOutputFile);
    cet::for_all(*outputWorkersToClose_.load(), closeFile);
    cet::for_all(*outputWorkersToClose_.load(), invoke_sPostCloseOutputFile);
    *outputWorkersToOpen_.load() = move(*outputWorkersToClose_.load());
  }

  // Called by EventProcessor::openSomeOutputFiles(), which is called when
  // output file switching is happening. Note: This really just returns
  // !outputWorkersToOpen_.load()->empty()
  bool
  EndPathExecutor::outputsToOpen() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    return !outputWorkersToOpen_.load()->empty();
  }

  // Called by EventProcessor::openSomeOutputFiles(), which is called when
  // output file switching is happening. Note this also calls:
  //   setOutputFileStatus(OutputFileStatus::Open);
  //   outputWorkersToOpen_.load()->clear();
  void
  EndPathExecutor::openSomeOutputFiles(FileBlock const& fb)
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    auto openFile = [&fb](auto ow) { ow->openFile(fb); };
    auto invoke_sPostOpenOutputFile = [this](auto ow) {
      actReg_.load()->sPostOpenOutputFile.invoke(ow->label());
    };
    cet::for_all(*outputWorkersToOpen_.load(), openFile);
    cet::for_all(*outputWorkersToOpen_.load(), invoke_sPostOpenOutputFile);
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
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->setFileStatus(ofs);
    }
    fileStatus_ = ofs;
  }

  // Note: What this is really used for is to push workers into
  //       the outputWorkersToClose_ data member.
  void
  EndPathExecutor::recordOutputClosureRequests(Granularity const atBoundary)
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      if (atBoundary < ow->fileGranularity()) {
        // The boundary we are checking at is finer than
        // the checks the output worker needs, nothing to do.
        continue;
      }
      // Ask the worker if it wants to close.
      auto wants_to_close = ow->requestsToCloseFile();
      if (wants_to_close) {
        outputWorkersToClose_.load()->insert(ow);
      }
    }
  }

  // Used by file switching.
  // Called by EventProcessor::closeInputFile()
  // What this really does is cause RootOutputFile to call
  // RootOutputClosingCriteria::update<Granularity::InputFile>() which counts
  // how many times we have crossed a file boundary.
  void
  EndPathExecutor::incrementInputFileNumber()
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
    for (auto ow : *outputWorkers_.load()) {
      ow->incrementInputFileNumber();
    }
  }

  // Called by EventProcessor::readAndProcessEventFunctor(...)
  // Return whether or not all of the output workers have
  // reached their maximum limit of work to do.
  bool
  EndPathExecutor::allAtLimit() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{ofsMutex_, __func__};
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
