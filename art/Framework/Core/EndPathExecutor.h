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
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/trim.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/WaitingTask.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <vector>

namespace art {
  class EventProcessor;
  class EndPathExecutor {
    friend class EventProcessor;
  public:
    // Special Member Functions
    ~EndPathExecutor();
    EndPathExecutor(PathManager& pm,
                    ActionTable const& actions,
                    ActivityRegistry const& areg,
                    UpdateOutputCallbacks& callbacks);
    EndPathExecutor(EndPathExecutor const&) = delete;
    EndPathExecutor(EndPathExecutor&&) = delete;
    EndPathExecutor& operator=(EndPathExecutor const&) = delete;
    EndPathExecutor& operator=(EndPathExecutor&&) = delete;

    // Check Range Validity
    void check();

    void beginJob();
    void endJob();

    // Input File Open/Close.
    void selectProducts(ProductTables const&);
    void respondToOpenInputFile(FileBlock const& fb);
    void respondToCloseInputFile(FileBlock const& fb);
    void respondToOpenOutputFiles(FileBlock const& fb);
    void respondToCloseOutputFiles(FileBlock const& fb);
    bool someOutputsOpen() const;
    void closeAllOutputFiles();

    void seedRunRangeSet(std::unique_ptr<RangeSetHandler>);
    void setRunAuxiliaryRangeSetID(RangeSet const& rs);
    void writeRun(RunPrincipal& rp);

    void seedSubRunRangeSet(std::unique_ptr<RangeSetHandler>);
    void setSubRunAuxiliaryRangeSetID(RangeSet const& rs);
    void writeSubRun(SubRunPrincipal& srp);

    // Process Run/SubRun
    void process(Transition, Principal&);

    // Process Event
    //
    // Used to make sure only one event is being processed at a time.
    // The schedules take turns having their events processed on a
    // first-come first-served basis (FIFO).
    template <typename T>
    void push(const T& func);
    void process_event(EventPrincipal&, ScheduleID const);
    void writeEvent(ScheduleID const, EventPrincipal&);

    // Output File Switching API
    //
    // Called by EventProcessor::closeSomeOutputFiles(), which is called when
    // output file switching is happening. Note: This is really returns
    // !outputWorkersToClose_.empty()
    bool outputsToClose() const;
    // MT note: This is where we need to get all the schedules
    //          synchronized, and then have all schedules do the file
    //          close, and then the file open, then the schedules can
    //          proceed. A nasty complication is that a great deal of
    //          time can go by between the file close and the file
    //          open because artdaq may pause the run in between, and
    //          wants to have all output files closed while the run is
    //          paused.  They probably want the input file closed too.
    void closeSomeOutputFiles();
    // Note: This really just returns !outputWorkersToOpen_.empty()
    bool outputsToOpen() const;
    void openSomeOutputFiles(FileBlock const& fb);
    // Note: When we are passed OutputFileStatus::Switching, we must close
    //       the file and call openSomeOutputFiles which changes it back
    //       to OutputFileStatus::Open.
    //       A side effect of switching status is the run/subrun/event writes
    //       are not counted in the overall counting by
    //       RootOutputClosingCriteria. However, they are still counted by the
    //       individual counters.
    void setOutputFileStatus(OutputFileStatus);
    // Note: What this is really used for is to push workers into
    //       the outputWorkersToClose_ data member.
    void recordOutputClosureRequests(Granularity);
    void incrementInputFileNumber();
    // Return whether or not all of the output workers have
    // reached their maximum limit of work to do.
    bool allAtLimit() const;

  private:
    // Protects runRangeSetHandler_, and subRunRangeSetHandler_.
    mutable hep::concurrency::RecursiveMutex mutex_{"EndPathExecutor::mutex_"};
    // Filled by ctor, const after that.
    std::atomic<ActionTable const*> actionTable_;
    std::atomic<ActivityRegistry const*> actReg_;
    std::atomic<PathsInfo*> endPathInfo_;
    // Dynamic, used to force only one event at a time to be active on the end
    // path.
    std::atomic<hep::concurrency::SerialTaskQueue*> serialTaskQueue_;
    // Dynamic, cause an error if more than one thread processes an event.
    std::atomic<int> runningWorkerCnt_;
    // Filled by ctor, const after that.
    std::atomic<std::vector<OutputWorker*>*> outputWorkers_;
    // Dynamic, updated by run processing.
    std::atomic<PerScheduleContainer<RangeSetHandler*>*> runRangeSetHandler_;
    // Dynamic, updated by subrun processing.
    std::atomic<PerScheduleContainer<RangeSetHandler*>*> subRunRangeSetHandler_;

    // Output File Switching
    // Protects fileStatus_, outputWorkersToOpen_, and outputWorkersToClose_
    mutable hep::concurrency::RecursiveMutex ofsMutex_{
      "EndPathExecutor::ofsMutex_"};
    std::atomic<OutputFileStatus> fileStatus_;
    std::atomic<std::set<OutputWorker*>*> outputWorkersToOpen_;
    // Note: During an output file switch, after the closes happen, the entire
    // contents of this is moved to outputWorkersToOpen_.
    // FIXME: The move to outputWorkersToOpen_ is not really necessary, a flag
    // is all we need, something that says whether we should close or open what
    // is in the list. Basically EventProcessor uses recordOutputClosureRequests
    // to populate the list, then uses the list to do closes, then uses the same
    // list to do opens, then clears the list.
    std::atomic<std::set<OutputWorker*>*> outputWorkersToClose_;
  };

  template <typename T>
  void
  EndPathExecutor::push(const T& func)
  {
    serialTaskQueue_.load()->push(func);
  }

} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_EndPathExecutor_h */
