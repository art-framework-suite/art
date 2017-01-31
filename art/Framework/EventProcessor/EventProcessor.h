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
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/EventProcessor/ServiceDirector.h"
#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/PassID.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "cetlib/exception.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "tbb/task_scheduler_init.h"

#include <memory>
#include <string>
#include <vector>

namespace statemachine {
  class Machine;
}

// ----------------------------------------------------------------------

namespace art {
  class EventProcessor;

  namespace detail {
    template <typename T>
    class PrincipalSignalSentry;
  }
}

class art::EventProcessor {
public:

  // Status codes:
  //   0     successful completion
  //   3     signal received
  //  values are for historical reasons.
  enum Status { epSuccess=0, epSignal=3 };

  // Eventually, we might replace StatusCode with a class. This
  // class should have an automatic conversion to 'int'.

  using StatusCode = Status;

  EventProcessor (EventProcessor const&) = delete;
  EventProcessor& operator=(EventProcessor const&) = delete;

  // The input string 'config' contains the entire contents of a  configuration file.
  // Also allows the attachement of pre-existing services specified  by 'token', and
  // the specification of services by name only (defaultServices and forcedServices).
  // 'defaultServices' are overridden by 'config'.
  // 'forcedServices' cause an exception if the same service is specified in 'config'.

  EventProcessor(fhicl::ParameterSet const& pset);
  ~EventProcessor();

  // This should be called before the first call to 'run'.
  void beginJob();

  // This should be called before the EventProcessor is destroyed
  // throws if any module's endJob throws an exception.
  void endJob();

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

  // The following functions are used by the code implementing our
  // boost statemachine

  void openInputFile();
  void closeAllFiles();
  void closeInputFile();
  void openAllOutputFiles();
  void closeAllOutputFiles();
  void openSomeOutputFiles();
  void closeSomeOutputFiles();

  void respondToOpenInputFile();
  void respondToCloseInputFile();
  void respondToOpenOutputFiles();
  void respondToCloseOutputFiles();

  void rewindInput();
  void incrementInputFileNumber();
  void recordOutputClosureRequests(Boundary);

  void doErrorStuff();

  template <Level L> void finalize();
  template <Level L> void try_finalize();
  template <Level L> void setExceptionMessage(std::string const&);

  void setupCurrentRun();
  void readRun();
  void beginRun();
  void beginRunIfNotDoneAlready();
  void setRunAuxiliaryRangeSetID();
  void endRun();
  void writeRun();
  void finalizeRun();

  void setupCurrentSubRun();
  void readSubRun();
  void beginSubRun();
  void beginSubRunIfNotDoneAlready();
  void setSubRunAuxiliaryRangeSetID();
  void endSubRun();
  void writeSubRun();
  void finalizeSubRun();

  void readEvent();
  void processEvent();
  void writeEvent();
  void finalizeEvent();

  // Run/SubRun IDs from most recently added principals
  RunID runPrincipalID() const;
  SubRunID subRunPrincipalID() const;
  EventID eventPrincipalID() const;

  bool shouldWeStop() const;

  bool alreadyHandlingException() const;

  bool switchInProgress() const { return switchInProgress_; }
  bool stagingAllowed() const { return stagingAllowed_; }

  void disallowStaging() {
    setOutputFileStatus(OutputFileStatus::StagedToSwitch);
    stagingAllowed_ = false;
  }

  bool outputsToOpen() const;
  bool outputsToClose() const;
  bool someOutputsOpen() const;
  void setOutputFileStatus(OutputFileStatus);
  void setStagingAllowed(bool const value) { stagingAllowed_ = value; }
  void setSwitchInProgress(bool const value) { switchInProgress_ = value; }
  void maybeTriggerOutputFileSwitch();

  bool setTriggerPathEnabled(std::string const& name, bool enable);
  bool setEndPathModuleEnabled(std::string const& label, bool enable);

  // Stuff from state machine
  bool beginRunCalled() const { return beginRunCalled_; }
  bool exitRunCalled() const { return exitRunCalled_; }
  bool runException() const { return runException_; }
  bool finalizeRunEnabled() const { return finalizeRunEnabled_; }

  bool beginSubRunCalled() const { return beginSubRunCalled_; }
  bool exitSubRunCalled() const { return exitSubRunCalled_; }
  bool subRunException() const { return subRunException_; }
  bool finalizeSubRunEnabled() const { return finalizeSubRunEnabled_; }

  bool exitEventCalled() const { return exitEventCalled_; }
  bool eventException() const { return eventException_; }
  bool finalizeEventEnabled() const { return finalizeEventEnabled_; }

  void setBeginRunCalled(bool const value) { beginRunCalled_ = value; }
  void setExitRunCalled(bool const value) { exitRunCalled_ = value; }
  void setRunException(bool const value) { runException_ = value; }
  void setFinalizeRunEnabled(bool const value) { finalizeRunEnabled_ = value; }

  void setBeginSubRunCalled(bool const value) { beginSubRunCalled_ = value; }
  void setExitSubRunCalled(bool const value) { exitSubRunCalled_ = value; }
  void setSubRunException(bool const value) { subRunException_ = value; }
  void setFinalizeSubRunEnabled(bool const value) { finalizeSubRunEnabled_ = value; }

  void setExitEventCalled(bool const value) { exitEventCalled_ = value; }
  void setEventException(bool const value) { eventException_ = value; }
  void setFinalizeEventEnabled(bool const value) { finalizeEventEnabled_ = value; }

private:

  template <Level L> std::enable_if_t<(underlying_value(L)<underlying_value(Level::N)-1)> begin();
  template <Level L> std::enable_if_t<(underlying_value(L)<underlying_value(Level::N)-1)> process();
  template <Level L> std::enable_if_t<(underlying_value(L)==underlying_value(Level::N)-1)> process();
  template <Level L> std::enable_if_t<(underlying_value(L)<underlying_value(Level::N)-1)> end();

  template <Level L> void endContainingLevels();


  template <Level L>
  bool levelsToProcess();

  void levelProcessed();
  Level advance();

  Level nextLevel_ {Level::Empty};
  std::vector<Level> activeLevels_ {Level::Job};

  // Stuff from state machine
  bool beginRunCalled_ {false}; // Should be stack variable local to run loop
  bool exitRunCalled_ {false};  // ""
  bool runException_ {false};   // ""
  bool finalizeRunEnabled_ {true};

  bool beginSubRunCalled_ {false}; // Should be stack variable local to run loop
  bool exitSubRunCalled_ {false};  // ""
  bool subRunException_ {false};   // ""
  bool finalizeSubRunEnabled_ {true};

  bool exitEventCalled_ {false};
  bool eventException_ {false};
  bool finalizeEventEnabled_ {true};

  ServiceDirector initServices_(fhicl::ParameterSet const& top_pset,
                                ActivityRegistry& areg,
                                ServiceToken& token);
  void initSchedules_(fhicl::ParameterSet const& pset);
  void invokePostBeginJobWorkers_();
  template <typename T>
  void
  process_(typename T::MyPrincipal& p);

  ServiceToken getToken_();

  StatusCode runCommon_();
  void servicesActivate_(ServiceToken st);
  void servicesDeactivate_();
  void terminateMachine_();
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
  MasterProductRegistry preg_ {};
  ServiceToken serviceToken_ {};
  tbb::task_scheduler_init tbbManager_ {tbb::task_scheduler_init::deferred};
  std::unique_ptr<ServiceRegistry::Operate> servicesSentry_ {};
  PathManager pathManager_; // Must outlive schedules.
  ServiceDirector serviceDirector_;
  std::unique_ptr<InputSource> input_ {nullptr};
  std::unique_ptr<Schedule> schedule_ {nullptr};
  std::unique_ptr<EndPathExecutor> endPathExecutor_ {nullptr};
  std::unique_ptr<FileBlock> fb_ {nullptr};

  std::unique_ptr<statemachine::Machine> machine_ {nullptr};
  std::unique_ptr<RunPrincipal> runPrincipal_ {nullptr};
  std::unique_ptr<SubRunPrincipal> subRunPrincipal_ {nullptr};
  std::unique_ptr<EventPrincipal> eventPrincipal_ {nullptr};
  bool shouldWeStop_ {false};
  bool stateMachineWasInErrorState_ {false};
  bool const handleEmptyRuns_;
  bool const handleEmptySubRuns_;
  bool stagingAllowed_ {true};
  bool switchInProgress_ {false};

  // Note that the non-const references are initialized after the
  // exceptionMessages_ object is initialized.
  std::array<std::string, art::underlying_value(art::Level::N)> exceptionMessages_;
  std::string& exceptionMessageFiles_ {std::get<art::underlying_value(art::Level::InputFile)>(exceptionMessages_)};
  std::string& exceptionMessageRuns_ {std::get<art::underlying_value(art::Level::Run)>(exceptionMessages_)};
  std::string& exceptionMessageSubRuns_ {std::get<art::underlying_value(art::Level::SubRun)>(exceptionMessages_)};
  std::string& exceptionMessageEvents_ {std::get<art::underlying_value(art::Level::Event)>(exceptionMessages_)};

  bool alreadyHandlingException_ {false};

};  // EventProcessor

////////////////////////////////////
// Class ScheduleSignalSentry<T> is used to emit the pre- and post-
// signals associated with the principal associated with T.
template <typename T>
class art::detail::PrincipalSignalSentry {
public:

  PrincipalSignalSentry(PrincipalSignalSentry<T> const&) = delete;
  PrincipalSignalSentry<T> operator=(PrincipalSignalSentry<T> const&) = delete;

  using principal_t = typename T::MyPrincipal;
  explicit PrincipalSignalSentry(art::ActivityRegistry& a, principal_t& ep);
  ~PrincipalSignalSentry();

private:
  art::ActivityRegistry& a_;
  principal_t& ep_;
};

template <class T>
art::detail::PrincipalSignalSentry<T>::
PrincipalSignalSentry(art::ActivityRegistry& a,
                      typename PrincipalSignalSentry<T>::principal_t& ep)
  :
  a_{a},
  ep_{ep}
{
  T::preScheduleSignal(&a_, &ep_);
}

template <class T>
art::detail::PrincipalSignalSentry<T>::
~PrincipalSignalSentry()
{
  T::postScheduleSignal(&a_, &ep_);
}

template <typename T>
void
art::EventProcessor::process_(typename T::MyPrincipal& p)
try {
  detail::PrincipalSignalSentry<T> sentry(actReg_, p);
  schedule_->process<T>(p);
  endPathExecutor_->process<T>(p);
}
catch (cet::exception & ex) {
  actions::ActionCodes const action {
    T::level == Level::Event ? act_table_.find(ex.root_cause()) : actions::Rethrow
  };
  switch (action) {
  case actions::IgnoreCompletely: {
    mf::LogWarning(ex.category())
      << "exception being ignored for current event:\n"
      << cet::trim_right_copy(ex.what(), " \n");
    break;
  }
  default: {
    throw art::Exception(errors::EventProcessorFailure)
      << "An exception occurred during current event processing\n"
      << ex;
  }
  }
}
catch (...) {
  mf::LogError("PassingThrough")
    << "an exception occurred during current event processing\n";
  throw;
}

template <art::Level L>
bool
art::EventProcessor::levelsToProcess()
{
  if (nextLevel_ == Level::Empty) {
    nextLevel_ = advance();
    boost::mutex::scoped_lock sl {usr2_lock};
    if (art::shutdown_flag > 0) {
      throw Exception{errors::SignalReceived};
    }
  }

  if (nextLevel_ == L) {
    activeLevels_.push_back(nextLevel_);
    nextLevel_ = Level::Empty;
    return true;
  }
  else if (nextLevel_ < L) {
    return false;
  }
  else if (nextLevel_ == Level::Done) {
    return false;
  }

  throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
}

namespace art {
  template <> inline void EventProcessor::begin<Level::Job>() { beginJob(); }
  template <> inline void EventProcessor::begin<Level::InputFile>() { openInputFile(); }
  template <> inline void EventProcessor::begin<Level::Run>() { setupCurrentRun(); }
  template <> inline void EventProcessor::begin<Level::SubRun>() { setupCurrentSubRun(); }

  template <> inline void EventProcessor::finalize<Level::Event>() { finalizeEvent(); }
  template <> inline void EventProcessor::finalize<Level::SubRun>() { finalizeSubRun(); }
  template <> inline void EventProcessor::finalize<Level::Run>() { finalizeRun(); }
  template <> inline void EventProcessor::finalize<Level::InputFile>() {
    if (nextLevel_ == Level::Done) {
      closeAllFiles();
    }
    else {
      closeInputFile();
    }
  }
  template <> inline void EventProcessor::finalize<Level::Job>() { endJob(); }

  template <> inline void EventProcessor::endContainingLevels<Level::Job>() {}
  template <> inline void EventProcessor::endContainingLevels<Level::InputFile>() {}
  template <> inline void EventProcessor::endContainingLevels<Level::Run>() {}

  template <> inline void EventProcessor::endContainingLevels<Level::SubRun>()
  {
    try_finalize<Level::Run>();
  }

  template <> inline void EventProcessor::endContainingLevels<Level::Event>()
  {
    try_finalize<Level::SubRun>();
    endContainingLevels<Level::SubRun>();
  }
}



template <art::Level L>
std::enable_if_t<(art::underlying_value(L)==art::underlying_value(art::Level::N)-1)>
art::EventProcessor::process()
{
  beginRunIfNotDoneAlready();
  beginSubRunIfNotDoneAlready();
  readEvent();

  if (eventPrincipalID().isFlush()) return;
  processEvent();

  if (shouldWeStop()) { // ???
  }
  try_finalize<Level::Event>();
  if (outputsToClose()) {
    disallowStaging();
    endContainingLevels<Level::Event>();
    closeSomeOutputFiles();
  }
}

template <art::Level L>
std::enable_if_t<(art::underlying_value(L)<art::underlying_value(art::Level::N)-1)>
art::EventProcessor::process()
{
  begin<L>();
  auto constexpr next_level = static_cast<art::Level>(art::underlying_value(L)+1);
  while (levelsToProcess<next_level>()) {
    process<next_level>();
    levelProcessed();
  }
  try_finalize<L>();
  if (outputsToClose()) {
    disallowStaging();
    endContainingLevels<L>();
    closeSomeOutputFiles();
  }
}

template <art::Level L>
void
art::EventProcessor::setExceptionMessage(std::string const& message)
{
  std::get<art::underlying_value(L)>(exceptionMessages_) = message;
}

template <art::Level L>
void
art::EventProcessor::try_finalize()
  try {
    finalize<L>();
  }
  catch (cet::exception const& e) {
    std::ostringstream message;
    message << "------------------------------------------------------------\n"
            << "Another exception was caught while trying to clean up " << L << "s after\n"
            << "the primary exception.  We give up trying to clean up " << L << "s at\n"
            << "this point.  The description of this additional exception follows:\n"
            << "cet::exception\n"
            << e.explain_self();
    setExceptionMessage<L>(message.str());
  }
  catch (std::bad_alloc const& e) {
    std::ostringstream message;
    message << "------------------------------------------------------------\n"
            << "Another exception was caught while trying to clean up " << L << "s\n"
            << "after the primary exception.  We give up trying to clean up " << L << "s\n"
            << "at this point.  This additional exception was a\n"
            << "std::bad_alloc exception thrown inside Handle" << L << "s::finalize" << L << ".\n"
            << "The job has probably exhausted the virtual memory available\n"
            << "to the process.\n";
    setExceptionMessage<L>(message.str());
  }
  catch (std::exception const& e) {
    std::ostringstream message;
    message << "------------------------------------------------------------\n"
            << "Another exception was caught while trying to clean up " << L << "s after\n"
            << "the primary exception.  We give up trying to clean up " << L << "s at\n"
            << "this point.  This additional exception was a\n"
            << "standard library exception thrown inside Handle" << L << "s::finalize" << L << "\n"
            << e.what() << "\n";
    setExceptionMessage<L>(message.str());
  }
  catch (...) {
    std::ostringstream message;
    message << "------------------------------------------------------------\n"
            << "Another exception was caught while trying to clean up " << L << "s after\n"
            << "the primary exception.  We give up trying to clean up " << L << "s at\n"
            << "this point.  This additional exception was of unknown type and\n"
            << "thrown inside Handle" << L << "s::finalize" << L << "\n";
    setExceptionMessage<L>(message.str());
  }

// ======================================================================

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
