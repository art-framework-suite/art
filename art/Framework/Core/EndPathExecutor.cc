#include "art/Framework/Core/EndPathExecutor.h"

#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/OutputFileInfo.h"
#include "cetlib/container_algorithms.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace {
  auto getOutputWorkers(art::WorkerMap const& workers)
  {
    std::vector<art::OutputWorker*> ows;
    for (auto const & val : workers) {
      auto w = val.second.get();
      if (auto ow = dynamic_cast<art::OutputWorker*>(w)) {
        ows.emplace_back(ow);
      }
    }
    return ows;
  }
}

art::EndPathExecutor::
EndPathExecutor(PathManager& pm,
                ActionTable& actions,
                ActivityRegistry& areg,
                MasterProductRegistry& mpr)
  : endPathInfo_{pm.endPathInfo()}
  , act_table_{&actions}
  , actReg_{areg}
  , outputWorkers_{getOutputWorkers(endPathInfo_.workers())}
  , outputWorkersToOpen_(std::begin(outputWorkers_), std::end(outputWorkers_)) // seed with all output workers
  , workersEnabled_(endPathInfo_.workers().size(), true)
  , outputWorkersEnabled_(outputWorkers_.size(), true)
{
  mpr.registerProductListUpdatedCallback([this](auto const& fb){ this->selectProducts(fb); });
}

bool art::EndPathExecutor::terminate() const
{
  bool const rc = !outputWorkers_.empty() && // Necessary because std::all_of()
                                             // returns true if range is empty.
                  std::all_of(outputWorkers_.cbegin(),
                              outputWorkers_.cend(),
                              [](auto& w){ return w->limitReached(); });
  if (rc) {
    mf::LogInfo("SuccessfulTermination")
      << "The job is terminating successfully because each output module\n"
      << "has reached its configured limit.\n";
  }
  return rc;
}

void
art::EndPathExecutor::
endJob()
{
  bool failure {false};
  Exception error {errors::EndJobFailure};
  doForAllEnabledWorkers_
    ([&failure, &error](Worker * w)
     {
       try {
         w->endJob();
       }
       catch (cet::exception & e) {
         error << "cet::exception caught in Schedule::endJob\n"
               << e.explain_self();
         failure = true;
       }
       catch (std::exception & e) {
         error << "Standard library exception caught in Schedule::endJob\n"
               << e.what();
         failure = true;
       }
       catch (...) {
         error << "Unknown exception caught in Schedule::endJob\n";
         failure = true;
       }
     });
  if (failure) { throw error; }
}

void art::EndPathExecutor::closeAllOutputFiles()
{
  doForAllEnabledOutputWorkers_([this](OutputWorker * ow) {
      actReg_.sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.sPostCloseOutputFile.invoke(OutputFileInfo(ow->label(),
                                                         ow->lastClosedFileName()));
    });
}

void art::EndPathExecutor::openAllOutputFiles(FileBlock & fb)
{
  doForAllEnabledOutputWorkers_([this, &fb](OutputWorker* ow) {
      ow->openFile(fb);
      actReg_.sPostOpenOutputFile.invoke(ow->label());
    });
}

void art::EndPathExecutor::writeRun(RunPrincipal& rp)
{
  doForAllEnabledOutputWorkers_([&rp](auto w){ w->writeRun(rp); });
  if (fileStatus_ == OutputFileStatus::Switching) {
    runRangeSetHandler_->rebase();
  }
}

void art::EndPathExecutor::writeSubRun(SubRunPrincipal& srp)
{
  doForAllEnabledOutputWorkers_([&srp](auto w){ w->writeSubRun(srp); });
  if (fileStatus_ == OutputFileStatus::Switching) {
    subRunRangeSetHandler_->rebase();
  }
}

void art::EndPathExecutor::writeEvent(EventPrincipal& ep)
{
  doForAllEnabledOutputWorkers_([&ep](auto w){ w->writeEvent(ep); });
  auto const& eid = ep.id();
  bool const lastInSubRun = ep.isLastInSubRun();
  runRangeSetHandler_->update(eid, lastInSubRun);
  subRunRangeSetHandler_->update(eid, lastInSubRun);
}

void art::EndPathExecutor::seedRunRangeSet(std::unique_ptr<RangeSetHandler> rsh)
{
  runRangeSetHandler_ = std::move(rsh);
}

void art::EndPathExecutor::seedSubRunRangeSet(std::unique_ptr<RangeSetHandler> rsh)
{
  subRunRangeSetHandler_ = std::move(rsh);
}

void art::EndPathExecutor::setAuxiliaryRangeSetID(SubRunPrincipal& srp)
{
  // Ranges are split/flushed only for a RangeSetHandler whose dynamic
  // type is 'ClosedRangeSetHandler'.  The implementations for the
  // 'OpenRangeSetHandler' are nops.
  //
  // Consider the following range-sets
  //  SubRun RangeSet: { Run 1 : SubRun 1 : Events [1,7) }  <-- Current iterator of handler
  //  Run    RangeSet: { Run 1 : SubRun 0 : Events [5,11)
  //                             SubRun 1 : Events [1,7)    <-- Current iterator of handler
  //                             SubRun 1 : Events [9,15) }
  if (fileStatus_ == OutputFileStatus::Switching) {
    // For a range split just before SubRun 1, Event 6, the range sets
    // should become:
    //
    //  SubRun RangeSet: { Run 1 : SubRun 1 : Events [1,6)
    //                             SubRun 1 : Events [6,7) } <-- Updated iterator of handler
    //  Run    RangeSet: { Run 1 : SubRun 0 : Events [5,11)
    //                             SubRun 1 : Events [1,6)
    //                             SubRun 1 : Events [6,7)   <-- Updated iterator of handler
    //                             SubRun 1 : Events [9,15) }
    subRunRangeSetHandler_->maybeSplitRange();
    runRangeSetHandler_->maybeSplitRange();
  }
  else {
    subRunRangeSetHandler_->flushRanges();
  }
  auto const& ranges = subRunRangeSetHandler_->seenRanges();
  srp.updateSeenRanges(ranges);
  doForAllEnabledOutputWorkers_([&ranges](auto w){ w->setSubRunAuxiliaryRangeSetID(ranges); });
}

void art::EndPathExecutor::setAuxiliaryRangeSetID(RunPrincipal& rp)
{
  if (fileStatus_ != OutputFileStatus::Switching) {
    runRangeSetHandler_->flushRanges();
  }
  auto const& ranges = runRangeSetHandler_->seenRanges();
  rp.updateSeenRanges(ranges);
  doForAllEnabledOutputWorkers_([&ranges](auto w){ w->setRunAuxiliaryRangeSetID(ranges); });
}

void art::EndPathExecutor::selectProducts(FileBlock const& fb)
{
  doForAllEnabledOutputWorkers_([&fb](auto w) { w->selectProducts(fb); });
}

void art::EndPathExecutor::recordOutputClosureRequests(Boundary const b)
{
  doForAllEnabledOutputWorkers_([this,b](auto ow) {
      // We need to support the following case:
      //
      //   fileProperties: {
      //     maxEvents: 10
      //     maxRuns: 1
      //     granularity: Event
      //   }
      //
      // If a request to close is made on a run boundary, but the
      // granularity is still Event, then the file should close.  For
      // that reason, the comparison is 'granularity > b' instead of
      // 'granularity != b'.

      auto const granularity = ow->fileSwitchBoundary();
      if (granularity > b || !ow->requestsToCloseFile()) return;

      // Technical note: although the outputWorkersToClose_ container
      // is "moved from" in closeSomeOutputFiles, it is safe to call
      // 'insert' vis-a-vis the [lib.types.movedfrom] section of the
      // standard.  There are no preconditions for std::set::insert,
      // so no state-checking is required.
      outputWorkersToClose_.insert(ow);
    });
}

void art::EndPathExecutor::incrementInputFileNumber()
{
  doForAllEnabledOutputWorkers_([](auto ow) {
      ow->incrementInputFileNumber();
    });
}

bool art::EndPathExecutor::outputsToOpen() const
{
  return !outputWorkersToOpen_.empty();
}

bool art::EndPathExecutor::outputsToClose() const
{
  return !outputWorkersToClose_.empty();
}

bool art::EndPathExecutor::someOutputsOpen() const
{
  return std::any_of(outputWorkers_.cbegin(),
                     outputWorkers_.cend(),
                     [](auto ow) {
                       return ow->fileIsOpen();
                     } );
}

void art::EndPathExecutor::closeSomeOutputFiles()
{
  auto invoke_sPreCloseOutputFile  = [this](auto ow){actReg_.sPreCloseOutputFile.invoke(ow->label());};
  auto closeFile                   = [    ](auto ow){ow->closeFile();};
  auto invoke_sPostCloseOutputFile = [this](auto ow){actReg_.sPostCloseOutputFile.invoke(OutputFileInfo{ow->label(), ow->lastClosedFileName()});};

  setOutputFileStatus(OutputFileStatus::Switching);
  cet::for_all(outputWorkersToClose_, invoke_sPreCloseOutputFile);
  cet::for_all(outputWorkersToClose_, closeFile);
  cet::for_all(outputWorkersToClose_, invoke_sPostCloseOutputFile);
  outputWorkersToOpen_ = std::move(outputWorkersToClose_);
}

void art::EndPathExecutor::setOutputFileStatus(OutputFileStatus const ofs)
{
  doForAllEnabledOutputWorkers_([ofs](auto ow){ow->setFileStatus(ofs);});
  fileStatus_ = ofs;
}

void art::EndPathExecutor::openSomeOutputFiles(FileBlock const& fb)
{
  auto openFile                    = [ &fb](auto ow){ow->openFile(fb);};
  auto invoke_sPostOpenOutputFile  = [this](auto ow){actReg_.sPostOpenOutputFile.invoke(ow->label());};

  cet::for_all(outputWorkersToOpen_, openFile);
  cet::for_all(outputWorkersToOpen_, invoke_sPostOpenOutputFile);
  setOutputFileStatus(OutputFileStatus::Open);

  outputWorkersToOpen_.clear();
}

void art::EndPathExecutor::respondToOpenInputFile(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToOpenInputFile(fb); });
}

void art::EndPathExecutor::respondToCloseInputFile(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToCloseInputFile(fb); });
}

void art::EndPathExecutor::respondToOpenOutputFiles(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToOpenOutputFiles(fb); });
}

void art::EndPathExecutor::respondToCloseOutputFiles(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToCloseOutputFiles(fb); });
}

void art::EndPathExecutor::beginJob()
{
  doForAllEnabledWorkers_([](auto w){ w->beginJob(); });
}

bool
art::EndPathExecutor::
setEndPathModuleEnabled(std::string const & label, bool enable)
{
  bool result;
  auto & workers = endPathInfo_.workers();
  WorkerMap::iterator foundW;
  if ((foundW = workers.find(label)) != workers.end()) {
    size_t const index = std::distance(workers.begin(), foundW);
    result = workersEnabled_[index];
    workersEnabled_[index] = enable;
  } else {
    throw Exception(errors::ScheduleExecutionFailure)
      << "Attempt to "
      << (enable?"enable":"disable")
      << " unconfigured module "
      << label
      << ".\n";
  }
  auto owFinder = [&label](OutputWorkers::const_reference ow) {
    return ow->label() == label;
  };
  OutputWorkers::iterator foundOW;
  if ((foundOW = std::find_if(outputWorkers_.begin(),
                              outputWorkers_.end(),
                              owFinder)) != outputWorkers_.end()) {
    auto const index = std::distance(outputWorkers_.begin(), foundOW);
    outputWorkersEnabled_[index] = enable;
  }
  return result;
}

void
art::EndPathExecutor::resetAll()
{
  doForAllEnabledWorkers_([](auto w){ w->reset(); });
}
