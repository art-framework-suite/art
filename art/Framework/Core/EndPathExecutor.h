#ifndef art_Framework_Core_EndPathExecutor_h
#define art_Framework_Core_EndPathExecutor_h
// vim: set sw=2 expandtab :

// =========================================================================
// Class to handle the execution of the end path. Invoked in all the
// right places by the event processor.
//
// The RangeSetHandlers manage the RangeSets that are to be assigned
// to (a) the (Sub)RunAuxiliaries and (b) the (Sub)Run products
// produced in the current process.  Since all (Sub)Run
// products/auxiliaries produced in the current process are written to
// all output modules during write(Sub)Run, there is only one relevant
// RangeSet for the (Sub)Run at any given time.  RangeSets
// corresponding to multiple (Sub)Run fragments are aggregated on
// input.
// =========================================================================

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/trim.h"
#include "hep_concurrency/WaitingTask.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <vector>

namespace art {

class EndPathExecutor {

public:

  EndPathExecutor(PathManager& pm, ActionTable& actions, ActivityRegistry& areg, MasterProductRegistry& mpr);

public: // MEMBER FUNCTIONS -- Begin/End Job

  // Called by EventProcessor::beginJob()
  void
  beginJob();

  // Called by EventProcessor::endJob()
  void
  endJob();

public: // MEMBER FUNCTIONS -- Input File Open/Close.

  // Called by MasterProductRegistry product list updaters (on input file open).
  void
  selectProducts(ProductLists const&);

  // Called by EventProcessor::openInputFile()
  //   Called by EventProcessor::begin<Level::InputFile>()
  void
  respondToOpenInputFile(FileBlock const& fb);

  // Called by EventProcessor::closeInputFile()
  //   Called by EventProcessor::finalize<Level::InputFile>()
  //   Called by EventProcessor::closeAllFiles()
  //     Called by EventProcessor::finalize<Level::InputFile>()
  void
  respondToCloseInputFile(FileBlock const& fb);

  // Called by EventProcessor::respondToOpenOutputFiles(
  //   Called by EventProcessor::openSomeOutputFiles() (output file switching)
  void
  respondToOpenOutputFiles(FileBlock const& fb);

  // Called by EventProcessor::respondToCloseOutputFiles()
  //   Called by EventProcessor::closeAllOutputFiles()
  //     Called by EventProcessor::closeAllFiles(),
  //       Called by EventProcessor::finalize<Level::InputFile>()
  //   Called by EventProcessor::closeSomeOutputFiles() (output file switching)
  void
  respondToCloseOutputFiles(FileBlock const& fb);

  // Called by EventProcessor::closeAllOutputFiles()
  //   Called by EventProcessor::closeAllFiles(),
  //     Called by EventProcessor::finalize<Level::InputFile>()
  bool
  someOutputsOpen() const;

  // Called by EventProcessor::closeAllOutputFiles()
  //   Called by EventProcessor::closeAllFiles(),
  //     Called by EventProcessor::finalize<Level::InputFile>()
  void
  closeAllOutputFiles();

  // FIXME: Can be deleted!
  // Called by EventProcessor::openAllOutputFiles(), which is never called.
  void
  openAllOutputFiles(FileBlock& fb);

public: // MEMBER FUNCTIONS -- Begin/End Run

  // Called only by EventProcessor::readRun()
  void
  seedRunRangeSet(std::unique_ptr<RangeSetHandler>);

  // Called by EventProcessor::setRunAuxiliaryRangeSetID(), which is called by EventProcessor::finalize<Level::Run>()
  void
  setAuxiliaryRangeSetID(RunPrincipal& rp);

  // Called by EventProcessor::writeRun(), which is called by EventProcessor::finalize<Level::Run>()
  void
  writeRun(RunPrincipal& rp);

public: // MEMBER FUNCTIONS -- Begin/End SubRun

  // Called only by EventProcessor::readSubRun()
  void
  seedSubRunRangeSet(std::unique_ptr<RangeSetHandler>);

  // Called by EventProcessor::setSubRunAuxiliaryRangeSetID(), which is called by EventProcessor::finalize<Level::SubRun>()
  void
  setAuxiliaryRangeSetID(SubRunPrincipal& srp);

  // Called by EventProcessor::writeSubRun(), which is called by EventProcessor::finalize<Level::SubRun>()
  void
  writeSubRun(SubRunPrincipal& srp);

public: // MEMBER FUNCTIONS -- Process Run/SubRun

  void
  process(Transition, Principal&);

public: // MEMBER FUNCTIONS -- Process Event

  // Called by EventProcessor::processAllEventsAsync_processEndPath(...)
  // Used to make sure only one event is being
  // processed at a time.  The streams take turns
  // having their events processed on a first-come
  // first-served basis (FIFO).
  hep::concurrency::SerialTaskQueue&
  serialTaskQueue();

  // Called by EventProcessor::processAllEventsAsync_processEndPath::endPathFunctor()
  void
  process_event(EventPrincipal&, int streamIndex);

  //FIXME: This is the old system that treated the end path
  //FIXME: in the same way as the trigger path.
  //FIXME: UNUSED, REMOVE!
  // Called by EventProcessor::readAndProcessEventFunctor(...)
  void
  process_event(hep::concurrency::WaitingTask* finishEventTask, EventPrincipal&, int streamIndex);

  // Called by EventProcessor::readAndProcessEventFunctor()
  void
  writeEvent(int streamIndex, EventPrincipal& ep);

public: // MEMBER FUNCTIONS -- Output File Switching API

  //
  //  See also:
  //
  //    respondToOpenOutputFiles(FileBlock const& fb);
  //    respondToCloseOutputFiles(FileBlock const& fb);
  //

  // Called by EventProcessor::closeSomeOutputFiles(), which is called when output file switching is happening.
  // Note: This is really returns !outputWorkersToClose_.empty()
  bool
  outputsToClose() const;

  // Called by EventProcessor::closeSomeOutputFiles(), which is called when output file switching is happening.
  // Note: threading: This is where we need to get all the streams
  // Note: threading: synchronized, and then have all streams do
  // Note: threading: the file close, and then the file open, then
  // Note: threading: the streams can proceed.
  // Note: threading: A nasty complication is that a great deal of
  // Note: threading: time can go by between the file close and the
  // Note: threading: file open because artdaq may pause the run
  // Note: threading: inbetween, and wants to have all output files
  // Note: threading: closed while the run is paused.  They probably
  // Note: threading: want the input file closed too.
  void
  closeSomeOutputFiles();

  // Called by EventProcessor::openSomeOutputFiles(), which is called when output file switching is happening.
  // Note: This really just returns !outputWorkersToOpen_.empty()
  bool
  outputsToOpen() const;

  // Called by EventProcessor::openSomeOutputFiles(), which is called when output file switching is happening.
  // Note this also calls:
  //   setOutputFileStatus(OutputFileStatus::Open);
  //   outputWorkersToOpen_.clear();
  void
  openSomeOutputFiles(FileBlock const& fb);

  // Note: When we are passed OutputFileStatus::Switching, we must close
  //       the file and call openSomeOutputFiles which changes it back
  //       to OutputFileStatus::Open.
  //       A side effect of switching status is the run/subrun/event writes
  //       are not counted in the overall counting by RootOutputClosingCriteria.
  //       However, they are still counted by the individual counters.
  void
  setOutputFileStatus(OutputFileStatus);

  // Note: What this is really used for is to push workers into
  //       the outputWorkersToClose_ data member.
  void
  recordOutputClosureRequests(Granularity);

  // Called by EventProcessor::closeInputFile()
  // What this really does is cause RootOutputFile to call RootOutputClosingCriteria::update<Granularity::InputFile>()
  // which counts how many times we have crossed a file boundary.
  void
  incrementInputFileNumber();

  // Called by EventProcessor::readAndProcessEventFunctor(...)
  // Return whether or not all of the output workers have
  // reached their maximum limit of work to do.
  bool
  allAtLimit() const;

private: // MEMBER FUNCTIONS -- Implementation details.

  //FIXME: This is the old system that treated the end path
  //FIXME: in the same way as the trigger path.
  //FIXME: UNUSED, REMOVE!
  void
  process_event_pathsDone(hep::concurrency::WaitingTask* finishEventTask, EventPrincipal&, int streamIndex);

private: // MEMBER DATA

  // Filled by ctor, const after that.
  ActionTable&
  actionTable_;

  // Filled by ctor, const after that.
  ActivityRegistry&
  actReg_;

  // Filled by ctor, const after that.
  PathsInfo&
  endPathInfo_;

  // Dynamic, used to force only one
  // event at a time to be active on
  // the end path.
  hep::concurrency::SerialTaskQueue
  serialTaskQueue_{};

  // Filled by ctor, const after that.
  std::vector<OutputWorker*>
  outputWorkers_{};

  // Dynamic, updated by run processing.
  // Note: threading: Will need to be protected when multiple runs/subruns in-flight is implemented.
  // Note: Indexed by streamIndex.
  std::vector<std::unique_ptr<RangeSetHandler>>
  runRangeSetHandler_{};

  // Dynamic, updated by subrun processing.
  // Note: threading: Will need to be protected when multiple runs/subruns in-flight is implemented.
  // Note: Indexed by streamIndex.
  std::vector<std::unique_ptr<RangeSetHandler>>
  subRunRangeSetHandler_{};

private: // MEMBER DATA -- Output File Switching

  // Dynamic, changed by EventProcessor (using setOutputFileStatus(OutputFileStatus))
  // to track whether or not we have entered output file switching mode.
  OutputFileStatus
  fileStatus_{OutputFileStatus::Closed};

  // Dynamic, updated internally by:
  //   closeSomeOutputFiles()
  //   outputsToOpen() const
  //   openSomeOutputFiles(FileBlock const& fb)
  // to manage the output file switches.
  std::set<OutputWorker*>
  outputWorkersToOpen_{};


  // Dynamic, updated internally by:
  //   outputsToClose() const
  //   closeSomeOutputFiles()
  //   recordOutputClosureRequests(Granularity const atBoundary)
  //     Called by EventProcessor::(various) at boundaries where switching can happen.
  // to manage the output file switches.
  // Note: During an output file switch, after the closes happen, the entire contents
  //       of this is moved to outputWorkersToOpen_.
  // FIXME: The move to outputWorkersToOpen_ is not really necessary, a flag is all we
  //        need, something that says whether we should close or open what is in the list.
  //        Basically EventProcessor uses recordOutputClosureRequests to populate the
  //        list, then uses the list to do closes, then uses the same list to do opens,
  //        then clears the list.
  std::set<OutputWorker*>
  outputWorkersToClose_{};

};

} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_EndPathExecutor_h */
