#ifndef art_Framework_EventProcessor_EventProcessor_h
#define art_Framework_EventProcessor_EventProcessor_h

// ======================================================================
//
// EventProcessor - This defines the 'framework application' object. It
// is configured in the user's main() function, and is set running.
//
// ======================================================================

#include "art/Framework/Core/EndPathExecutor.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/MFStatusUpdater.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/ProducingServiceSignals.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/EventProcessor/ServiceDirector.h"
#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "cetlib/cpu_timer.h"
#include "cetlib/trim.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "tbb/task_scheduler_init.h"

#include <memory>
#include <string>
#include <vector>

namespace art {
  class EventProcessor;
}

class art::EventProcessor {
public:
  // Status codes:
  //   0     successful completion
  //   3     signal received
  //  values are for historical reasons.
  enum Status { epSuccess = 0, epSignal = 3 };

  // Eventually, we might replace StatusCode with a class. This
  // class should have an automatic conversion to 'int'.

  using StatusCode = Status;

  EventProcessor(EventProcessor const&) = delete;
  EventProcessor& operator=(EventProcessor const&) = delete;

  explicit EventProcessor(fhicl::ParameterSet const& pset);
  ~EventProcessor();

  //------------------------------------------------------------------
  // The function "runToCompletion" will run until the job is "complete",
  // which means:
  //       1 - no more input data
  //       2 - input maxEvents parameter limit reached
  //       3 - output maxEvents parameter limit reached
  //       4 - input maxSubRuns parameter limit reached
  //
  // The return values from runToCompletion are as follows:
  //   epSignal - processing terminated early, SIGUSR2 encountered
  //   epSuccess - all other cases
  //
  StatusCode runToCompletion();

private:
  // Event-loop infrastructure
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

  void markLevelAsProcessed();
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

  bool shouldWeStop() const;

  void setOutputFileStatus(OutputFileStatus);

  Level nextLevel_{Level::ReadyToAdvance};
  std::vector<Level> activeLevels_{highest_level()};
  detail::ExceptionCollector ec_{};
  cet::cpu_timer timer_{};

  bool beginRunCalled_{false}; // Should be stack variable local to run loop
  bool beginSubRunCalled_{
    false}; // Should be stack variable local to subrun loop

  bool finalizeRunEnabled_{true};
  bool finalizeSubRunEnabled_{true};

  ServiceDirector initServices_(fhicl::ParameterSet const& top_pset,
                                ActivityRegistry& areg,
                                ServiceToken& token);
  void initSchedules_(fhicl::ParameterSet const& pset);
  void invokePostBeginJobWorkers_();

  template <typename T>
  void process_(typename T::MyPrincipal& p);

  void servicesActivate_(ServiceToken st);
  void servicesDeactivate_();
  void terminateAbnormally_();

  //------------------------------------------------------------------
  //
  // Data members below.
  // Are all these data members really needed? Some of them are used
  // only during construction, and never again. If they aren't
  // really needed, we should remove them.

  ActionTable act_table_;
  ActivityRegistry actReg_;
  MFStatusUpdater mfStatusUpdater_;
  MasterProductRegistry preg_{};
  ProductDescriptions productsToProduce_{};
  ProducingServiceSignals psSignals_{};
  ServiceToken serviceToken_{ServiceToken::createInvalid()};
  tbb::task_scheduler_init tbbManager_{tbb::task_scheduler_init::deferred};
  std::unique_ptr<ServiceRegistry::Operate> servicesSentry_{};
  PathManager pathManager_; // Must outlive schedules.
  ServiceDirector serviceDirector_;
  std::unique_ptr<InputSource> input_{nullptr};
  std::unique_ptr<Schedule> schedule_{nullptr};
  std::unique_ptr<EndPathExecutor> endPathExecutor_{nullptr};
  std::unique_ptr<FileBlock> fb_{nullptr};

  std::unique_ptr<RunPrincipal> runPrincipal_{nullptr};
  std::unique_ptr<SubRunPrincipal> subRunPrincipal_{nullptr};
  std::unique_ptr<EventPrincipal> eventPrincipal_{nullptr};
  ProductTables producedProducts_{ProductTables::invalid()};
  bool shouldWeStop_{false};
  bool const handleEmptyRuns_;
  bool const handleEmptySubRuns_;

}; // EventProcessor

////////////////////////////////////
template <typename T>
void
art::EventProcessor::process_(typename T::MyPrincipal& p) try {
  T::preScheduleSignal(actReg_, p);
  schedule_->process<T>(p);
  endPathExecutor_->process<T>(p);
  T::postScheduleSignal(actReg_, p);
}
catch (cet::exception const& ex) {
  actions::ActionCodes const action{T::level == Level::Event ?
                                      act_table_.find(ex.root_cause()) :
                                      actions::Rethrow};
  switch (action) {
    case actions::IgnoreCompletely: {
      mf::LogWarning(ex.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(ex.what(), " \n");
      break;
    }
    default: {
      throw art::Exception{
        errors::EventProcessorFailure,
        "EventProcessor: an exception occurred during current event processing",
        ex};
    }
  }
}
catch (...) {
  mf::LogError("PassingThrough")
    << "an exception occurred during current event processing\n";
  throw;
}

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
