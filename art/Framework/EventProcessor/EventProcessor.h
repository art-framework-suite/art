#ifndef art_Framework_EventProcessor_EventProcessor_h
#define art_Framework_EventProcessor_EventProcessor_h
// vim: set sw=2 expandtab :

//
//  The art framework master controller.
//

#include "art/Framework/Core/EndPathExecutor.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/MFStatusUpdater.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/ProducingServiceSignals.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/Core/UpdateOutputCallbacks.h"
#include "art/Framework/EventProcessor/Scheduler.h"
#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "cetlib/cpu_timer.h"
#include "cetlib/trim.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task_scheduler_init.h"

#include <atomic>
#include <exception>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace cet {
  class ostream_handle;
} // namespace cet

namespace art {

  class EventProcessor {
  public: // TYPES
    // Status codes:
    //   0     successful completion
    //   3     signal received
    //  values are for historical reasons.
    enum Status { epSuccess = 0, epSignal = 3 };

    using StatusCode = Status;

    // MEMBER FUNCTIONS -- Special Member Functions
  public:
    ~EventProcessor();
    explicit EventProcessor(fhicl::ParameterSet const& pset);
    EventProcessor(EventProcessor const&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(EventProcessor const&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;

    // MEMBER FUNCTIONS -- API we provide to run_art
  public:
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

    // MEMBER FUNCTIONS -- Tasking Structure
  public:
    void processAllEventsTask(tbb::task* eventLoopTask,
                              ScheduleID const,
                              std::exception_ptr const*);
    void readAndProcessEventTask(tbb::task* eventLoopTask,
                                 ScheduleID const,
                                 std::exception_ptr const*);
    void endPathTask(tbb::task* eventLoopTask,
                     ScheduleID const,
                     std::exception_ptr const*);
    void endPathRunnerTask(ScheduleID const, tbb::task* eventLoopTask);

  private: // MEMBER FUCNTIONS -- Event Loop Infrastructure
    // Event-loop infrastructure
    void processAllEventsAsync(tbb::task* EventLoopTask, ScheduleID const);
    void readAndProcessAsync(tbb::task* EventLoopTask, ScheduleID const);
    void processEventAsync(tbb::task* EventLoopTask, ScheduleID const);
    void processEndPathAsync(tbb::task* EventLoopTask, ScheduleID const);
    void finishEventAsync(tbb::task* eventLoopTask, ScheduleID const);
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
    void openInputFile();
    void openSomeOutputFiles();
    void openAllOutputFiles();
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

  private: // MEMBER DATA
    template <typename T>
    class thread_sanitize {
    public:
      template <typename... Args>
      thread_sanitize(Args&&... args)
      {
        obj_ = new T(std::forward<Args>(args)...);
      }

      operator T&() { return *obj_.load(); }

      thread_sanitize&
      operator=(T&& t)
      {
        *obj_.load() = std::forward<T>(t);
        return *this;
      }

      T* operator->() const { return obj_.load(); }

      ~thread_sanitize() noexcept
      {
        ANNOTATE_THREAD_IGNORE_BEGIN;
        delete obj_.load();
        obj_ = nullptr;
        ANNOTATE_THREAD_IGNORE_END;
      }

    private:
      std::atomic<T*> obj_;
    };

    // Next containment level to move to.
    std::atomic<Level> nextLevel_{Level::ReadyToAdvance};

    // Utility object to run a functor and collect any exceptions thrown.
    thread_sanitize<detail::ExceptionCollector> ec_{};

    // Used for timing the job.
    thread_sanitize<cet::cpu_timer> timer_{};

    // Used to keep track of whether or not we have already call beginRun.
    std::atomic<bool> beginRunCalled_{false};

    // Used to keep track of whether or not we have already call beginSubRun.
    std::atomic<bool> beginSubRunCalled_{false};

    // When set allows runs to end.
    std::atomic<bool> finalizeRunEnabled_{true};

    // When set allows subruns to end.
    std::atomic<bool> finalizeSubRunEnabled_{true};

    // A signal/slot system for registering a callback to be called
    // when a specific action is taken by the framework.
    thread_sanitize<ActivityRegistry> actReg_{};

    // Used to update various output fields in logged messages.
    thread_sanitize<MFStatusUpdater> mfStatusUpdater_{actReg_};

    // List of callbacks which, when invoked, can update the state of
    // any output modules.
    // FIXME: Used only in the ctor!
    thread_sanitize<UpdateOutputCallbacks> outputCallbacks_{};

    // Product descriptions for the products that appear in
    // produces<T>() clauses in modules. Note that this is the master
    // copy and must be kept alive until producedProductLookupTables_
    // is destroyed because it has references to us.
    thread_sanitize<ProductDescriptions> producedProductDescriptions_{};

    // Product lookup tables for the products that appear in
    // produces<T>() clauses in modules. Note that this also serves as
    // the master list of produced products and must be kept alive
    // until no more principals that might use it exist. Also note
    // that we keep references to the internals of
    // producedProductDescriptions_.
    thread_sanitize<ProductTables> producedProductLookupTables_{
      ProductTables::invalid()};

    thread_sanitize<ProducingServiceSignals> psSignals_{};

    // The service subsystem.
    std::atomic<ServicesManager*> servicesManager_;

    // The entity that manages all configuration data from the
    // services.scheduler block and (eventually) sets up the TBB task
    // scheduler.
    thread_sanitize<Scheduler> scheduler_;

    // Despite the name, this is what parses the paths and modules in
    // the FHiCL file and creates and owns them.
    std::atomic<PathManager*> pathManager_;

    // The source of input data.
    std::atomic<InputSource*> input_;

    // The trigger path runners.
    std::atomic<PerScheduleContainer<Schedule>*> schedule_;

    // The end path runner.
    std::atomic<EndPathExecutor*> endPathExecutor_;

    // The currently open primary input file.
    std::atomic<FileBlock*> fb_;

    // The currently active RunPrincipal.
    std::atomic<RunPrincipal*> runPrincipal_;

    // The currently active SubRunPrincipal.
    std::atomic<SubRunPrincipal*> subRunPrincipal_;

    // The currently active EventPrincipals.
    std::atomic<PerScheduleContainer<EventPrincipal*>*> eventPrincipal_;

    // Are we configured to process empty runs?
    std::atomic<bool> handleEmptyRuns_;

    // Are we configured to process empty subruns?
    std::atomic<bool> handleEmptySubRuns_;

    // Has an exception been captured already?
    std::atomic<bool> deferredExceptionPtrIsSet_{false};

    // An exception captured from event processing.
    std::atomic<std::exception_ptr*> deferredExceptionPtr_;

    // Set to true for the first event in a subRun to signal
    // that we should not advance to the next entry.
    // Note that this is shared in common between all the
    // schedules. This is onnly needed because we cannot peek ahead
    // to see that the next entry is an event, we actually must
    // advance to it before we can know.
    std::atomic<bool> firstEvent_{true};

    // Are we current switching output files?
    std::atomic<bool> fileSwitchInProgress_{false};
  };

} // namespace art

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
