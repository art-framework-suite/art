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
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/OutputFileInfo.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/DebugMacros.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/trim.h"

#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {

  EndPathExecutor::EndPathExecutor(PathManager& pm,
                                   ActionTable& actionTable,
                                   ActivityRegistry& areg,
                                   UpdateOutputCallbacks& outputCallbacks)
    : actionTable_{actionTable}
    , actReg_{areg}
    , endPathInfo_{pm.endPathInfo()}
    , serialTaskQueue_{}
    , outputWorkers_{}
    , runRangeSetHandler_()
    , subRunRangeSetHandler_()
    // Output File Switching API from here on down
    , fileStatus_{OutputFileStatus::Closed}
    , outputWorkersToOpen_{}
    , outputWorkersToClose_{}
  {
    runRangeSetHandler_.resize(Globals::instance()->streams());
    subRunRangeSetHandler_.resize(Globals::instance()->streams());
    for (auto const& val : endPathInfo_.workers()) {
      auto w = val.second;
      auto owp = dynamic_cast<OutputWorker*>(w);
      if (owp != nullptr) {
        outputWorkers_.emplace_back(owp);
      }
    }
    outputWorkersToOpen_.insert(outputWorkers_.cbegin(), outputWorkers_.cend());
    outputCallbacks.registerCallback(
      [this](auto const& tables) { this->selectProducts(tables); });
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End Job
  //

  void
  EndPathExecutor::beginJob()
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->beginJob();
    }
  }

  void
  EndPathExecutor::endJob()
  {
    Exception error{errors::EndJobFailure};
    // FIXME: There seems to be little value-added by the catch and rethrow
    // here.
    for (auto& label_and_worker : endPathInfo_.workers()) {
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
    for (auto ow : outputWorkers_) {
      ow->selectProducts(tables);
    }
  }

  void
  EndPathExecutor::respondToOpenInputFile(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->respondToOpenInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->respondToCloseInputFile(fb);
    }
  }

  void
  EndPathExecutor::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->respondToOpenOutputFiles(fb);
    }
  }

  void
  EndPathExecutor::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->respondToCloseOutputFiles(fb);
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
  EndPathExecutor::openAllOutputFiles(FileBlock& fb)
  {
    for (auto ow : outputWorkers_) {
      ow->openFile(fb);
      actReg_.sPostOpenOutputFile.invoke(ow->label());
    }
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End Run
  //

  void
  EndPathExecutor::seedRunRangeSet(unique_ptr<RangeSetHandler> rsh)
  {
    for (auto& uptr_rsh : runRangeSetHandler_) {
      uptr_rsh.reset(rsh->clone());
    }
  }

  void
  EndPathExecutor::setAuxiliaryRangeSetID(RunPrincipal& rp)
  {
    if (runRangeSetHandler_.at(0)->type() ==
        RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the streams have seen.
      RangeSet mergedSeenRanges;
      // int idx = 0;
      for (auto& uptr_rsh : runRangeSetHandler_) {
        // ostringstream buf;
        // buf << "-----> EndPathExecutor::setAuxiliaryRangeSetID(rp): " << idx
        // << " " << uptr_rsh->seenRanges() << "\n";  try {
        mergedSeenRanges.merge(uptr_rsh->seenRanges());
        //}
        // catch (...) {
        // cerr << "-----> EndPathExecutor::setAuxiliaryRangeSetID(rp):
        // EXCEPTION\n";  TDEBUG(5) << buf.str();  cerr << buf.str();  throw;
        //}
        //++idx;
      }
      rp.updateSeenRanges(mergedSeenRanges);
      for (auto ow : outputWorkers_) {
        ow->setRunAuxiliaryRangeSetID(mergedSeenRanges);
      }
      return;
    }
    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges, use the first one.
    // handler with the largest event number, that will be the
    // one which we will use as the file switch boundary.  Note
    // that is may not match the exactly the stream that triggered
    // the switch.  Do we need to fix this?
    unique_ptr<RangeSetHandler> rshAtSwitch{runRangeSetHandler_.at(0)->clone()};
    if (fileStatus_ != OutputFileStatus::Switching) {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    rp.updateSeenRanges(rshAtSwitch->seenRanges());
    for (auto ow : outputWorkers_) {
      ow->setRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
    }
  }

  void
  EndPathExecutor::writeRun(RunPrincipal& rp)
  {
    for (auto ow : outputWorkers_) {
      ow->writeRun(rp);
    }
    if (fileStatus_ == OutputFileStatus::Switching) {
      for (auto& uptr_rsh : runRangeSetHandler_) {
        uptr_rsh->rebase();
      }
    }
  }

  //
  //  MEMBER FUNCTIONS -- Begin/End SubRun
  //

  void
  EndPathExecutor::seedSubRunRangeSet(unique_ptr<RangeSetHandler> rsh)
  {
    for (auto& uptr_rsh : subRunRangeSetHandler_) {
      uptr_rsh.reset(rsh->clone());
    }
  }

  // Called by EventProcessor::finalize<Level::SubRun>()
  // just before endSubRun() and writeSubRun().
  void
  EndPathExecutor::setAuxiliaryRangeSetID(SubRunPrincipal& srp)
  {
    if (subRunRangeSetHandler_.at(0)->type() ==
        RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the streams have seen.
      RangeSet mergedRS;
      // int idx = 0;
      for (auto& uptr_rsh : subRunRangeSetHandler_) {
        // ostringstream buf;
        // buf << "-----> EndPathExecutor::setAuxiliaryRangeSetID(srp): " << idx
        // << " " << uptr_rsh->seenRanges() << "\n";  try {
        mergedRS.merge(uptr_rsh->seenRanges());
        //}
        // catch (...) {
        // cerr << "-----> EndPathExecutor::setAuxiliaryRangeSetID(srp):
        // EXCEPTION\n";  TDEBUG(5) << buf.str();  cerr << buf.str();  throw;
        //}
        //++idx;
      }
      srp.updateSeenRanges(mergedRS);
      for (auto ow : outputWorkers_) {
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
    //  SubRun RangeSet: { Run 1 : SubRun 1 : Events [1,7) }  <-- Current
    //  iterator of handler Run    RangeSet: { Run 1 : SubRun 0 : Events [5,11)
    //                             SubRun 1 : Events [1,7)    <-- Current
    //                             iterator of handler SubRun 1 : Events [9,15)
    //                             }
    //
    // For a range split just before SubRun 1, Event 6, the range sets should
    // become:
    //
    //  SubRun RangeSet: { Run 1 : SubRun 1 : Events [1,6)
    //                             SubRun 1 : Events [6,7) } <-- Updated
    //                             iterator of handler
    //  Run    RangeSet: { Run 1 : SubRun 0 : Events [5,11)
    //                             SubRun 1 : Events [1,6)
    //                             SubRun 1 : Events [6,7)   <-- Updated
    //                             iterator of handler SubRun 1 : Events [9,15)
    //                             }
    //
    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges.  Find the closed range set
    // handler with the largest event number, that will be the
    // one which we will use as the file switch boundary.  Note
    // that is may not match the exactly the stream that triggered
    // the switch.  Do we need to fix this?
    unsigned largestEvent = 1U;
    int idxOfMax = 0;
    int idx = 0;
    for (auto& uptr_rsh : subRunRangeSetHandler_) {
      auto rsh = dynamic_cast<ClosedRangeSetHandler*>(uptr_rsh.get());
      if (rsh->eventInfo().id().event() > largestEvent) {
        largestEvent = rsh->eventInfo().id().event();
        idxOfMax = idx;
      }
      ++idx;
    }
    unique_ptr<RangeSetHandler> rshAtSwitch{
      subRunRangeSetHandler_.at(idxOfMax)->clone()};
    if (fileStatus_ == OutputFileStatus::Switching) {
      rshAtSwitch->maybeSplitRange();
      unique_ptr<RangeSetHandler> runRSHAtSwitch{
        runRangeSetHandler_.at(idxOfMax)->clone()};
      runRSHAtSwitch->maybeSplitRange();
      for (auto& uptr_rsh : runRangeSetHandler_) {
        uptr_rsh.reset(runRSHAtSwitch->clone());
      }
    } else {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    for (auto& uptr_rsh : subRunRangeSetHandler_) {
      uptr_rsh.reset(rshAtSwitch->clone());
    }
    srp.updateSeenRanges(rshAtSwitch->seenRanges());
    for (auto ow : outputWorkers_) {
      // For RootOutput this enters the possibly split
      // range set into the range set db.
      ow->setSubRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
    }
  }

  void
  EndPathExecutor::writeSubRun(SubRunPrincipal& srp)
  {
    for (auto ow : outputWorkers_) {
      ow->writeSubRun(srp);
    }
    if (fileStatus_ == OutputFileStatus::Switching) {
      for (auto& uptr_rsh : subRunRangeSetHandler_) {
        uptr_rsh->rebase();
      }
    }
  }

  //
  //  MEMBER FUNCTIONS -- Process Non-Event
  //

  void
  EndPathExecutor::process(Transition trans, Principal& principal)
  {
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->reset(0);
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

  //
  //  MEMBER FUNCTIONS -- Process Event
  //

  hep::concurrency::SerialTaskQueue&
  EndPathExecutor::serialTaskQueue()
  {
    return serialTaskQueue_;
  }

  // Note: We come here as part of the endPath task, our
  // parent task is the eventLoop task.
  void
  EndPathExecutor::process_event(EventPrincipal& ep, int si)
  {
    TDEBUG(4) << "-----> Begin EndPathExecutor::process_event (" << si
              << ") ...\n";
    for (auto& label_and_worker : endPathInfo_.workers()) {
      auto& w = label_and_worker.second;
      w->reset(si);
    }
    endPathInfo_.incrementTotalEventCount();
    try {
      if (!endPathInfo_.paths().empty()) {
        endPathInfo_.paths().front()->process_event_for_endpath(ep, si);
      }
    }
    catch (cet::exception& ex) {
      // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
      // FailPath
      auto const action{actionTable_.find(ex.root_cause())};
      if (action != actions::IgnoreCompletely) {
        // Possible actions: Rethrow, SkipEvent, FailModule, FailPath
        TDEBUG(4) << "-----> End   EndPathExecutor::process_event (" << si
                  << ") ... because of EXCEPTION\n";
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
      TDEBUG(4) << "-----> End   EndPathExecutor::process_event (" << si
                << ") ... because of EXCEPTION\n";
      throw;
    }
    endPathInfo_.incrementPassedEventCount();
    TDEBUG(4) << "-----> End   EndPathExecutor::process_event (" << si
              << ") ...\n";
  }

  void
  EndPathExecutor::writeEvent(int si, EventPrincipal& ep)
  {
    for (auto ow : outputWorkers_) {
      auto const& md = ow->description();
      // FIXME: this is overkill.  Users just need to be able to
      // access the correct stream index within the service.  They do
      // not need a full-fledged context.
      CurrentProcessingContext cpc{si, nullptr, -1, false};
      detail::CPCSentry sentry{cpc};
      actReg_.sPreWriteEvent.invoke(md);
      ow->writeEvent(ep);
      actReg_.sPostWriteEvent.invoke(md);
    }
    auto const& eid = ep.eventID();
    bool const lastInSubRun{ep.isLastInSubRun()};
    {
      ostringstream buf;
      buf << "-----> EndPathExecutor::writeEvent: si: (";
      buf << si;
      buf << ") eid: ";
      buf << eid.run();
      buf << ", ";
      buf << eid.subRun();
      buf << ", ";
      buf << eid.event();
      buf << "\n";
      TDEBUG(5) << buf.str();
    }
    runRangeSetHandler_.at(si)->update(eid, lastInSubRun);
    subRunRangeSetHandler_.at(si)->update(eid, lastInSubRun);
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
  // !outputWorkersToClose_.empty()
  bool
  EndPathExecutor::outputsToClose() const
  {
    return !outputWorkersToClose_.empty();
  }

  // Called by EventProcessor::closeSomeOutputFiles(), which is called when
  // output file switching is happening. Note: threading: This is where we need
  // to get all the streams Note: threading: synchronized, and then have all
  // streams do Note: threading: the file close, and then the file open, then
  // Note: threading: the streams can proceed.
  // Note: threading: A nasty complication is that a great deal of
  // Note: threading: time can go by between the file close and the
  // Note: threading: file open because artdaq may pause the run
  // Note: threading: inbetween, and wants to have all output files
  // Note: threading: closed while the run is paused.  They probably
  // Note: threading: want the input file closed too.
  void
  EndPathExecutor::closeSomeOutputFiles()
  {
    auto invoke_sPreCloseOutputFile = [this](auto ow) {
      actReg_.sPreCloseOutputFile.invoke(ow->label());
    };
    auto closeFile = [](auto ow) { ow->closeFile(); };
    auto invoke_sPostCloseOutputFile = [this](auto ow) {
      actReg_.sPostCloseOutputFile.invoke(
        OutputFileInfo{ow->label(), ow->lastClosedFileName()});
    };
    setOutputFileStatus(OutputFileStatus::Switching);
    cet::for_all(outputWorkersToClose_, invoke_sPreCloseOutputFile);
    cet::for_all(outputWorkersToClose_, closeFile);
    cet::for_all(outputWorkersToClose_, invoke_sPostCloseOutputFile);
    outputWorkersToOpen_ = move(outputWorkersToClose_);
  }

  // Called by EventProcessor::openSomeOutputFiles(), which is called when
  // output file switching is happening. Note: This really just returns
  // !outputWorkersToOpen_.empty()
  bool
  EndPathExecutor::outputsToOpen() const
  {
    return !outputWorkersToOpen_.empty();
  }

  // Called by EventProcessor::openSomeOutputFiles(), which is called when
  // output file switching is happening. Note this also calls:
  //   setOutputFileStatus(OutputFileStatus::Open);
  //   outputWorkersToOpen_.clear();
  void
  EndPathExecutor::openSomeOutputFiles(FileBlock const& fb)
  {
    auto openFile = [&fb](auto ow) { ow->openFile(fb); };
    auto invoke_sPostOpenOutputFile = [this](auto ow) {
      actReg_.sPostOpenOutputFile.invoke(ow->label());
    };
    cet::for_all(outputWorkersToOpen_, openFile);
    cet::for_all(outputWorkersToOpen_, invoke_sPostOpenOutputFile);
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

  // Note: What this is really used for is to push workers into
  //       the outputWorkersToClose_ data member.
  void
  EndPathExecutor::recordOutputClosureRequests(Granularity const atBoundary)
  {
    for (auto ow : outputWorkers_) {
      if (atBoundary < ow->fileGranularity()) {
        // The boundary we are checking at is finer than
        // the checks the output worker needs, nothing to do.
        continue;
      }
      // Ask the worker if it wants to close.
      auto wants_to_close = ow->requestsToCloseFile();
      if (wants_to_close) {
        outputWorkersToClose_.insert(ow);
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
    for (auto ow : outputWorkers_) {
      ow->incrementInputFileNumber();
    }
  }

  // Called by EventProcessor::readAndProcessEventFunctor(...)
  // Return whether or not all of the output workers have
  // reached their maximum limit of work to do.
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
