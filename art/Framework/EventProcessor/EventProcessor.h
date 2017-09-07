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
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/Transition.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "cetlib/cpu_timer.h"
#include "cetlib/exception.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task_scheduler_init.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace art {

class EventProcessor {

public: // TYPES

  enum Status {
      epSuccess = 0 // successful completion
    , epSignal = 3 // signal received
  };

  using StatusCode = Status;

public: // MEMBER FUNCTIONS -- Special Member Functions

  ~EventProcessor();

  explicit
  EventProcessor(fhicl::ParameterSet const& pset);

  EventProcessor(EventProcessor const&) = delete;

  EventProcessor(EventProcessor&&) = delete;

  EventProcessor&
  operator=(EventProcessor const&) = delete;

  EventProcessor&
  operator=(EventProcessor&&) = delete;

public: // MEMBER FUNCTIONS -- API we provide to run_art

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
  StatusCode
  runToCompletion();

private: // MEMBER FUCNTIONS -- Event Loop Infrastructure

  // Event-loop infrastructure

  void
  processAllEventsAsync(int streamIndex);

  void
  processAllEventsAsync_readAndProcess(int streamIndex);

  //void
  //processAllEventsAsync_readAndProcess_do_file_switch(hep::concurrency::WaitingTask* resumeStreamTask, int streamIndex);

  //void
  //processAllEventsAsync_readAndProcess_after_output_switch(int streamIndex);

  void
  processAllEventsAsync_readAndProcess_after_possible_output_switch(int streamIndex);

  void
  processAllEventsAsync_processEvent(int streamIndex);

  void
  processAllEventsAsync_processEndPath(int streamIndex);

  void
  processAllEventsAsync_finishEvent(int streamIndex);

  template <Level L>
  bool
  levelsToProcess();

  template <Level L>
  std::enable_if_t<is_above_most_deeply_nested_level(L)>
  begin();

  template <Level L>
  void
  process();

  template <Level L>
  void
  finalize();

  template <Level L>
  void
  finalizeContainingLevels() {}

  template <Level L>
  void
  recordOutputModuleClosureRequests() {}

  Level
  advanceItemType();

  // Level-specific member functions
  void
  beginJob();

  void
  endJob();

  void
  openInputFile();

  void
  openSomeOutputFiles();

  void
  openAllOutputFiles();

  void
  closeInputFile();

  void
  closeSomeOutputFiles();

  void
  closeAllOutputFiles();

  void
  closeAllFiles();

  void
  respondToOpenInputFile();

  void
  respondToCloseInputFile();

  void
  respondToOpenOutputFiles();

  void
  respondToCloseOutputFiles();

  void
  readRun();

  void
  beginRun();

  void
  beginRunIfNotDoneAlready();

  void
  setRunAuxiliaryRangeSetID();

  void
  endRun();

  void
  writeRun();

  void
  readSubRun();

  void
  beginSubRun();

  void
  beginSubRunIfNotDoneAlready();

  void
  setSubRunAuxiliaryRangeSetID();

  void
  endSubRun();

  void
  writeSubRun();

  void
  readEvent();

  void
  processEvent();

  void
  writeEvent();

  void
  setOutputFileStatus(OutputFileStatus);

  ServicesManager*
  initServices_(fhicl::ParameterSet const& top_pset, ActivityRegistry& areg);

  void
  invokePostBeginJobWorkers_();

  void
  terminateAbnormally_();

private: // MEMBER DATA

  Level
  nextLevel_{Level::ReadyToAdvance};

  detail::ExceptionCollector
  ec_{};

  cet::cpu_timer
  timer_{};

  bool
  beginRunCalled_{false};

  bool
  beginSubRunCalled_{false};

  bool
  finalizeRunEnabled_{true};

  bool
  finalizeSubRunEnabled_{true};

  tbb::task_scheduler_init
  tbbManager_{tbb::task_scheduler_init::deferred};

  // A table of responses to be taken on reception
  // of thrown exceptions.
  ActionTable
  act_table_{};

  // A signal/slot system for registering a callback
  // to be called when a specific action is taken by
  // the framework.
  ActivityRegistry
  actReg_{};

  // Access to the MessageFacility (Marc Fischler).
  MFStatusUpdater
  mfStatusUpdater_{actReg_};

  // Master list of BranchDescriptions, indexed
  // by BranchKey.
  // Filled by the product list read from  input
  // files and by produces() calls.
  MasterProductRegistry
  mpr_{};

  // The service subsystem.
  std::unique_ptr<ServicesManager>
  servicesManager_{};

  // Despite the name, this is what parses the paths
  // and modules in the fcl file and creates and
  // owns them.
  PathManager
  pathManager_;

  // The source of input data.
  std::unique_ptr<InputSource>
  input_{};

  // Handles trigger paths.
  std::vector<Schedule>
  schedule_{};

  // Handles the end path.
  std::unique_ptr<EndPathExecutor>
  endPathExecutor_{};

  std::unique_ptr<FileBlock>
  fb_{};

  // Note: threading: This will need to be a vector when we implement multiple runs in flight.
  std::unique_ptr<RunPrincipal>
  runPrincipal_{};

  // Note: threading: This will need to be a vector when we implement multiple subruns in flight.
  std::unique_ptr<SubRunPrincipal>
  subRunPrincipal_{};

  // Note: threading: This will need to be a vector when we implement streams.
  std::vector<std::unique_ptr<EventPrincipal>>
  eventPrincipal_{};

  bool const
  handleEmptyRuns_;

  bool const
  handleEmptySubRuns_;

  std::atomic<bool>
  deferredExceptionPtrIsSet_{false};

  std::exception_ptr
  deferredExceptionPtr_;

  // Set to true for the first event in a subRun to signal
  // that we should not advance to the next entry.
  // Note that this is shared in common between all the
  // streams.
  // FIXME: Only needed because we cannot peek ahead to see
  // that the next entry is an event, we actually must advance
  // to it before we can know.
  std::atomic<bool>
  firstEvent_{true};

  std::atomic<bool>
  fileSwitchInProgress_{false};

  // Used to count the number of tasks waiting for an
  // output file switch to complete.
  std::atomic<int>
  waitingTaskCount_{};

  std::mutex
  mutexForCondFileSwitch_{};

  std::condition_variable
  condFileSwitch_{};

  // Used to hold the tasks that will resume the streams
  // after an output file switch.
  hep::concurrency::WaitingTaskList
  waitingTasks_{};

};

} // namespace art

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
