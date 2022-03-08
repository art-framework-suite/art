#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h
// vim: set sw=2 expandtab :

// ======================================================================
// Schedule
//
// A schedule contains all trigger paths and the end path executor for
// a stream of events.
//
// Processing of an event happens by pushing the event through the
// Paths. The scheduler performs the reset() on each of the workers
// independent of the Path objects.
// ======================================================================

#include "art/Framework/Core/EndPathExecutor.h"
#include "art/Framework/Core/TriggerPathsExecutor.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/EventPrincipal.h"

#include <atomic>
#include <cassert>
#include <memory>
#include <utility>

namespace art {
  class ActivityRegistry;
  namespace detail {
    class SharedResources;
  }

  class Schedule {
  public:
    Schedule(ScheduleID sid,
             PathManager& pm,
             ActionTable const& actions,
             ActivityRegistry const& aReg,
             UpdateOutputCallbacks& outputCallbacks,
             GlobalTaskGroup& task_group);

    // Disable copy/move operations
    Schedule(Schedule const&) = delete;
    Schedule(Schedule&&) = delete;
    Schedule& operator=(Schedule const&) = delete;
    Schedule& operator=(Schedule&&) = delete;

    // API presented to EventProcessor
    void process(Transition, Principal&);
    void process_event_modifiers(hep::concurrency::WaitingTaskPtr endPathTask);
    void process_event_observers(
      hep::concurrency::WaitingTaskPtr finalizeEventTask);
    void beginJob(detail::SharedResources const& resources);
    void endJob();
    void respondToOpenInputFile(FileBlock const&);
    void respondToCloseInputFile(FileBlock const&);
    void respondToOpenOutputFiles(FileBlock const&);
    void respondToCloseOutputFiles(FileBlock const&);

    // End-path API
    bool
    outputsToOpen() const
    {
      return epExec_.outputsToOpen();
    }

    bool
    outputsToClose() const
    {
      return epExec_.outputsToClose();
    }

    void
    recordOutputClosureRequests(Granularity const granularity)
    {
      return epExec_.recordOutputClosureRequests(granularity);
    }

    bool
    someOutputsOpen() const
    {
      return epExec_.someOutputsOpen();
    }

    void
    closeAllOutputFiles()
    {
      epExec_.closeAllOutputFiles();
    }

    void
    openSomeOutputFiles(FileBlock const& fb)
    {
      epExec_.openSomeOutputFiles(fb);
    }

    void
    closeSomeOutputFiles()
    {
      epExec_.closeSomeOutputFiles();
    }

    void
    writeEvent()
    {
      assert(eventPrincipal_);
      epExec_.writeEvent(*eventPrincipal_);
      // Delete principal
      eventPrincipal_.reset();
    }

    void
    incrementInputFileNumber()
    {
      epExec_.incrementInputFileNumber();
    }

    void
    setOutputFileStatus(OutputFileStatus const ofs)
    {
      epExec_.setOutputFileStatus(ofs);
    }

    OutputFileStatus
    fileStatus() const
    {
      return epExec_.fileStatus_.load();
    }

    // Run level
    void
    seedRunRangeSet(RangeSetHandler const& rsh)
    {
      epExec_.seedRunRangeSet(rsh);
    }

    void
    setRunAuxiliaryRangeSetID(RangeSet const& rs)
    {
      epExec_.setRunAuxiliaryRangeSetID(rs);
    }

    void
    writeRun(RunPrincipal& rp)
    {
      epExec_.writeRun(rp);
    }

    RangeSetHandler const&
    runRangeSetHandler()
    {
      return *epExec_.runRangeSetHandler_.get();
    }

    // SubRun level
    void
    seedSubRunRangeSet(RangeSetHandler const& rsh)
    {
      epExec_.seedSubRunRangeSet(rsh);
    }
    void
    setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
    {
      epExec_.setSubRunAuxiliaryRangeSetID(rs);
    }
    void
    writeSubRun(SubRunPrincipal& srp)
    {
      epExec_.writeSubRun(srp);
    }

    RangeSetHandler const&
    subRunRangeSetHandler()
    {
      return *epExec_.subRunRangeSetHandler_.get();
    }

    void
    accept_principal(std::unique_ptr<EventPrincipal> principal)
    {
      assert(principal);
      eventPrincipal_ = std::move(principal);
    }

    EventPrincipal&
    event_principal()
    {
      assert(eventPrincipal_);
      return *eventPrincipal_;
    }

    class EndPathRunnerTask;

  private:
    ScheduleContext const context_;
    ActionTable const& actions_;
    EndPathExecutor epExec_;
    TriggerPathsExecutor tpsExec_;
    std::unique_ptr<EventPrincipal> eventPrincipal_{nullptr};
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_Schedule_h */
