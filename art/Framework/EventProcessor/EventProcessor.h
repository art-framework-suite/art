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

  void beginRun();
  void endRun();

  void beginSubRun();
  void endSubRun();

  RunID    readRun();
  SubRunID readSubRun();

  void writeRun();
  void writeSubRun();
  void writeEvent();

  void setRunAuxiliaryRangeSetID();
  void setSubRunAuxiliaryRangeSetID();

  // Run/SubRun IDs from most recently added principals
  RunID runPrincipalID() const;
  SubRunID subRunPrincipalID() const;
  EventID eventPrincipalID() const;

  EventID readEvent();
  void processEvent();
  bool shouldWeStop() const;

  void setExceptionMessageFiles(std::string const& message);
  void setExceptionMessageRuns(std::string const& message);
  void setExceptionMessageSubRuns(std::string const& message);
  bool alreadyHandlingException() const;

  bool outputsToOpen() const;
  bool outputsToClose() const;
  bool someOutputsOpen() const;
  void setOutputFileStatus(OutputFileStatus);

  bool setTriggerPathEnabled(std::string const& name, bool enable);
  bool setEndPathModuleEnabled(std::string const& label, bool enable);

private:
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
  bool handleEmptyRuns_;
  bool handleEmptySubRuns_;
  std::string exceptionMessageFiles_ {};
  std::string exceptionMessageRuns_ {};
  std::string exceptionMessageSubRuns_ {};
  bool alreadyHandlingException_ {false};

};  // EventProcessor

////////////////////////////////////
// Class ScheduleSignalSentry<T> is used to emit the pre- and post-
// signals associated with the principal associated with T.
template <typename T>
class art::detail::PrincipalSignalSentry {
public:
  PrincipalSignalSentry(PrincipalSignalSentry<T> const &) = delete;
  PrincipalSignalSentry<T> operator=(PrincipalSignalSentry<T> const &) = delete;

  using principal_t = typename T::MyPrincipal;
  PrincipalSignalSentry(art::ActivityRegistry & a, principal_t & ep);
  ~PrincipalSignalSentry();

private:
  art::ActivityRegistry & a_;
  principal_t & ep_;
};

template <class T>
art::detail::PrincipalSignalSentry<T>::
PrincipalSignalSentry(art::ActivityRegistry & a,
                      typename PrincipalSignalSentry<T>::principal_t & ep)
  :
  a_(a),
  ep_(ep)
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


// ======================================================================

#endif /* art_Framework_EventProcessor_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
