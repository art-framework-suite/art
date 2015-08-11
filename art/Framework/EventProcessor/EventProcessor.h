#ifndef art_Framework_EventProcessor_EventProcessor_h
#define art_Framework_EventProcessor_EventProcessor_h

// ======================================================================
//
// EventProcessor - This defines the 'framework application' object. It
// is configured in the user's main() function, and is set running.
//
// ======================================================================

#include "art/Framework/Core/EndPathExecutor.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/MFStatusUpdater.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PrincipalCache.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/EventProcessor/EvProcInitHelper.h"
#include "art/Framework/EventProcessor/ServiceDirector.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "boost/thread/condition.hpp"
#include "boost/thread/thread.hpp"
#include "cetlib/exception.h"
#include "cetlib/trim.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "tbb/task_scheduler_init.h"

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

class art::EventProcessor : public art::IEventProcessor {
public:
  EventProcessor(EventProcessor const &) = delete;
  EventProcessor & operator=(EventProcessor const &) = delete;

  // The input string 'config' contains the entire contents of a  configuration file.
  // Also allows the attachement of pre-existing services specified  by 'token', and
  // the specification of services by name only (defaultServices and forcedServices).
  // 'defaultServices' are overridden by 'config'.
  // 'forcedServices' cause an exception if the same service is specified in 'config'.

  EventProcessor(fhicl::ParameterSet const & pset);
  ~EventProcessor();

  /**This should be called before the first call to 'run'.
       */
  void beginJob();

  /**This should be called before the EventProcessor is destroyed
       throws if any module's endJob throws an exception.
       */
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
  StatusCode runToCompletion() override;

  // The following functions are used by the code implementing our
  // boost statemachine

  void readFile() override;
  void closeInputFile() override;
  void openOutputFiles() override;
  void closeOutputFiles() override;

  void respondToOpenInputFile() override;
  void respondToCloseInputFile() override;
  void respondToOpenOutputFiles() override;
  void respondToCloseOutputFiles() override;

  void startingNewLoop() override;
  bool endOfLoop() override;
  void rewindInput() override;
  void prepareForNextLoop() override;
  void writeSubRunCache() override;
  void writeRunCache() override;
  bool shouldWeCloseOutput() const override;

  void doErrorStuff() override;

  void beginRun(RunID run) override;
  void endRun(RunID run) override;

  void beginSubRun(SubRunID const & sr) override;
  void endSubRun(SubRunID const & sr) override;

  RunID readAndCacheRun() override;
  SubRunID readAndCacheSubRun() override;
  void writeRun(RunID run) override;
  void deleteRunFromCache(RunID run) override;
  void writeSubRun(SubRunID const & sr) override;
  void deleteSubRunFromCache(SubRunID const & sr) override;

  void readEvent() override;
  void processEvent() override;
  bool shouldWeStop() const override;

  void setExceptionMessageFiles(std::string & message) override;
  void setExceptionMessageRuns(std::string & message) override;
  void setExceptionMessageSubRuns(std::string & message) override;
  bool alreadyHandlingException() const override;

  bool setTriggerPathEnabled(std::string const & name, bool enable) override;
  bool setEndPathModuleEnabled(std::string const & label, bool enable) override;

private:
  ServiceDirector initServices_(fhicl::ParameterSet const & top_pset,
                                ActivityRegistry & areg,
                                ServiceToken & token);
  void initSchedules_(fhicl::ParameterSet const & pset);
  void invokePostBeginJobWorkers_();
  template <typename T>
  void
  processOneOccurrence_(typename T::MyPrincipal & p);

  ServiceToken getToken_();

  StatusCode runCommon_();
  void terminateMachine_();
  void terminateAbnormally_();

  //------------------------------------------------------------------
  //
  // Data members below.
  // Are all these data members really needed? Some of them are used
  // only during construction, and never again. If they aren't
  // really needed, we should remove them.

  EvProcInitHelper helper_;
  ActionTable act_table_;
  ActivityRegistry actReg_;
  MFStatusUpdater mfStatusUpdater_;
  MasterProductRegistry preg_;
  ServiceToken serviceToken_;
  tbb::task_scheduler_init tbbManager_;
  // destructorOperate_ should be populated in destructor only!
  std::unique_ptr<ServiceRegistry::Operate> destructorOperate_;
  PathManager pathManager_; // Must outlive schedules.
  ServiceDirector serviceDirector_;
  std::unique_ptr<InputSource> input_;
  std::unique_ptr<Schedule> schedule_;
  std::unique_ptr<EndPathExecutor> endPathExecutor_;

  std::shared_ptr<FileBlock> fb_;

  std::unique_ptr<statemachine::Machine> machine_;
  PrincipalCache principalCache_;
  std::unique_ptr<EventPrincipal> sm_evp_;
  bool shouldWeStop_;
  bool stateMachineWasInErrorState_;
  std::string fileMode_;
  bool handleEmptyRuns_;
  bool handleEmptySubRuns_;
  std::string exceptionMessageFiles_;
  std::string exceptionMessageRuns_;
  std::string exceptionMessageSubRuns_;
  bool alreadyHandlingException_;
};  // EventProcessor

////////////////////////////////////
// Class ScheduleSignalSentry<T> is used to emit the pre- and post-
// signals associated with the principal associated with T.
template <typename T>
class art::detail::PrincipalSignalSentry {
public:
  PrincipalSignalSentry(PrincipalSignalSentry<T> const &) = delete;
  PrincipalSignalSentry<T> operator=(PrincipalSignalSentry<T> const &) = delete;

  typedef typename T::MyPrincipal principal_t;
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
art::EventProcessor::processOneOccurrence_(typename T::MyPrincipal & p)
try {
  detail::PrincipalSignalSentry<T> sentry(actReg_, p);
  schedule_->processOneOccurrence<T>(p);
  endPathExecutor_->processOneOccurrence<T>(p);
}
catch (cet::exception & ex) {
  actions::ActionCodes action = (T::isEvent_ ? act_table_.find(
                                   ex.root_cause()) : actions::Rethrow);
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
