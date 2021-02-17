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
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <vector>

namespace art {
  class EndPathExecutor {
    friend class Schedule;

  public:
    EndPathExecutor(ScheduleID sid,
                    PathManager& pm,
                    ActionTable const& actions,
                    ActivityRegistry const& areg,
                    UpdateOutputCallbacks& callbacks);

    EndPathExecutor(EndPathExecutor&&) = delete;
    EndPathExecutor& operator=(EndPathExecutor&&) = delete;
    EndPathExecutor(EndPathExecutor const&) = delete;
    EndPathExecutor& operator=(EndPathExecutor const&) = delete;

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

    void seedRunRangeSet(RangeSetHandler const&);
    void setRunAuxiliaryRangeSetID(RangeSet const& rs);
    void writeRun(RunPrincipal& rp);

    void seedSubRunRangeSet(RangeSetHandler const&);
    void setSubRunAuxiliaryRangeSetID(RangeSet const& rs);
    void writeSubRun(SubRunPrincipal& srp);

    // Process Run/SubRun
    void process(Transition, Principal&);

    // Process Event
    //
    // Used to make sure only one event is being processed at a time.
    // The schedules take turns having their events processed on a
    // first-come first-served basis (FIFO).
    void process_event(task_ptr_t finalizeEventTask, EventPrincipal&);
    void writeEvent(EventPrincipal&);

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
    class PathsDoneTask;

    // Filled by ctor, const after that.
    ScheduleContext const sc_;
    ActionTable const& actionTable_;
    ActivityRegistry const& actReg_;
    PathsInfo& endPathInfo_;
    // Filled by ctor, const after that.
    std::vector<OutputWorker*> outputWorkers_{};
    // Dynamic, updated by run processing.
    std::unique_ptr<RangeSetHandler> runRangeSetHandler_{nullptr};
    // Dynamic, updated by subrun processing.
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler_{nullptr};

    // Output File Switching
    std::atomic<OutputFileStatus> fileStatus_{OutputFileStatus::Closed};
    std::set<OutputWorker*> outputWorkersToOpen_{};
    // Note: During an output file switch, after the closes happen, the entire
    // contents of this is moved to outputWorkersToOpen_.
    // FIXME: The move to outputWorkersToOpen_ is not really necessary, a flag
    // is all we need, something that says whether we should close or open what
    // is in the list. Basically EventProcessor uses recordOutputClosureRequests
    // to populate the list, then uses the list to do closes, then uses the same
    // list to do opens, then clears the list.
    std::set<OutputWorker*> outputWorkersToClose_{};
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_EndPathExecutor_h */
