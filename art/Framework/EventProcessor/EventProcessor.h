#ifndef art_Framework_EventProcessor_EventProcessor_h
#define art_Framework_EventProcessor_EventProcessor_h
// vim: set sw=2 expandtab :

// ===========================
// The art application object.
// ===========================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/MFStatusUpdater.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/ProducingServiceSignals.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/Core/SharedException.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/Core/detail/EnabledModules.h"
#include "art/Framework/EventProcessor/Scheduler.h"
#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/ScheduleIteration.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "cetlib/cpu_timer.h"
#include "fhiclcpp/fwd.h"
#include "hep_concurrency/thread_sanitize.h"

#include <atomic>
#include <exception>
#include <memory>

namespace art {

  class EventProcessor {
  public:
    // Status codes:
    //   0     successful completion
    //   3     signal received
    //  values are for historical reasons.
    enum StatusCode { epSuccess = 0, epSignal = 3 };

    // Special Member Functions
    ~EventProcessor();
    explicit EventProcessor(fhicl::ParameterSet const& pset,
                            detail::EnabledModules const& enabled_modules);
    EventProcessor(EventProcessor const&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(EventProcessor const&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;

    // API to run_art
    //
    //  Run the job until done, which means:
    //
    //    - no more input data, or
    //    - input maxEvents parameter limit reached, or
    //    - output maxEvents parameter limit reached, or
    //    - input maxSubRuns parameter limit reached.
    //
    //  Return values:
    //
    //     epSignal: processing terminated early, SIGUSR2 encountered
    //    epSuccess: all other cases
    //
    StatusCode runToCompletion();

  private:
    class ProcessAllEventsTask;
    class EndPathTask;
    class EndPathRunnerTask;

    // Event-loop infrastructure
    void processAllEventsAsync(ScheduleID sid);
    void readAndProcessAsync(ScheduleID sid);
    void processEventAsync(ScheduleID sid);
    void finishEventAsync(ScheduleID sid);

    template <Level L>
    bool levelsToProcess();
    template <Level L>
    std::enable_if_t<is_above_most_deeply_nested_level(L)> begin();
    template <Level L>
    void process();
    template <Level L>
    void finalize();
    template <Level L>
    void
    finalizeContainingLevels()
    {}
    template <Level L>
    void
    recordOutputModuleClosureRequests()
    {}
    Level advanceItemType();

    // Level-specific member functions
    void beginJob();
    void endJob();
    void endJobAllSchedules();
    void openInputFile();
    bool outputsToOpen();
    void openSomeOutputFiles();
    void closeInputFile();
    void closeSomeOutputFiles();
    void closeAllOutputFiles();
    void closeAllFiles();
    void respondToOpenInputFile();
    void respondToCloseInputFile();
    void respondToOpenOutputFiles();
    void respondToCloseOutputFiles();
    void readRun();
    void beginRun();
    void beginRunIfNotDoneAlready();
    void setRunAuxiliaryRangeSetID();
    void endRun();
    void writeRun();
    void readSubRun();
    void beginSubRun();
    void beginSubRunIfNotDoneAlready();
    void setSubRunAuxiliaryRangeSetID();
    void endSubRun();
    void writeSubRun();
    void readEvent();
    void processEvent();
    void writeEvent();
    void setOutputFileStatus(OutputFileStatus);
    void invokePostBeginJobWorkers_();
    void terminateAbnormally_();

  private:
    template <typename T>
    using tsan = hep::concurrency::thread_sanitize<T>;

    template <typename T>
    using tsan_unique_ptr = hep::concurrency::thread_sanitize_unique_ptr<T>;

    Schedule&
    schedule(ScheduleID const id)
    {
      return schedules_->at(id);
    }

    Schedule&
    main_schedule()
    {
      return schedule(ScheduleID::first());
    }

    actions::ActionCodes
    error_action(cet::exception& e) const
    {
      return scheduler_->actionTable().find(e.root_cause());
    }

    // Next containment level to move to.
    std::atomic<Level> nextLevel_{Level::ReadyToAdvance};

    // Utility object to run a functor and collect any exceptions thrown.
    tsan<detail::ExceptionCollector> ec_{};

    // Used for timing the job.
    tsan<cet::cpu_timer> timer_{};

    // Used to keep track of whether or not we have already call beginRun.
    std::atomic<bool> beginRunCalled_{false};

    // Used to keep track of whether or not we have already call beginSubRun.
    std::atomic<bool> beginSubRunCalled_{false};

    // When set allows runs to end.
    std::atomic<bool> finalizeRunEnabled_{false};

    // When set allows subruns to end.
    std::atomic<bool> finalizeSubRunEnabled_{false};

    // A signal/slot system for registering a callback to be called
    // when a specific action is taken by the framework.
    ActivityRegistry actReg_{};

    // Used to update various output fields in logged messages.
    tsan<MFStatusUpdater> mfStatusUpdater_{actReg_};

    // List of callbacks which, when invoked, can update the state of
    // any output modules.
    // FIXME: Used only in the ctor!
    tsan<UpdateOutputCallbacks> outputCallbacks_{};

    // Product descriptions for the products that appear in
    // produces<T>() clauses in modules. Note that this is the master
    // copy and must be kept alive until producedProductLookupTables_
    // is destroyed because it has references to us.
    tsan<ProductDescriptions> producedProductDescriptions_{};

    // Product lookup tables for the products that appear in
    // produces<T>() clauses in modules. Note that this also serves as
    // the master list of produced products and must be kept alive
    // until no more principals that might use it exist. Also note
    // that we keep references to the internals of
    // producedProductDescriptions_.
    tsan<ProductTables> producedProductLookupTables_{ProductTables::invalid()};

    tsan<ProducingServiceSignals> psSignals_{};

    // The entity that manages all configuration data from the
    // services.scheduler block and (eventually) sets up the TBB task
    // scheduler.
    tsan<Scheduler> scheduler_;

    ScheduleIteration scheduleIteration_;

    // The service subsystem.
    tsan_unique_ptr<ServicesManager> servicesManager_;

    // Despite the name, this is what parses the paths and modules in
    // the FHiCL file and creates and owns them.
    tsan<PathManager> pathManager_;

    // The source of input data.
    tsan_unique_ptr<InputSource> input_{nullptr};

    // The schedule runners.
    tsan<std::map<ScheduleID, Schedule>> schedules_{};

    // The currently open primary input file.
    tsan_unique_ptr<FileBlock> fb_{nullptr};

    // The currently active RunPrincipal.
    tsan_unique_ptr<RunPrincipal> runPrincipal_{nullptr};

    // The currently active SubRunPrincipal.
    tsan_unique_ptr<SubRunPrincipal> subRunPrincipal_{nullptr};

    // The currently active EventPrincipals.
    PerScheduleContainer<std::unique_ptr<EventPrincipal>> eventPrincipals_{};

    // Are we configured to process empty runs?
    bool const handleEmptyRuns_;

    // Are we configured to process empty subruns?
    bool const handleEmptySubRuns_;

    // Used to communicate exceptions from worker threads to the main
    // thread.
    SharedException sharedException_;

    // Set to true for the first event in a subRun to signal that we
    // should not advance to the next entry.  Note that this is shared
    // in common between all the schedules. This is only needed
    // because we cannot peek ahead to see that the next entry is an
    // event, we actually must advance to it before we can know.
    std::atomic<bool> firstEvent_{true};

    // Are we current switching output files?
    std::atomic<bool> fileSwitchInProgress_{false};
  };

} // namespace art

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
