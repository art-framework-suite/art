#ifndef art_Framework_Core_EventProcessor_h
#define art_Framework_Core_EventProcessor_h

// ======================================================================
//
// EventProcessor - This defines the 'framework application' object. It
// is configured in the user's main() function, and is set running.
//
// ======================================================================

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/MFStatusUpdater.h"
#include "art/Framework/Core/PrincipalCache.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Framework/Core/WorkerRegistry.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "boost/thread/condition.hpp"
#include "boost/thread/thread.hpp"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "tbb/task_scheduler_init.h"

#include <string>
#include <vector>

namespace statemachine {
  class Machine;
}

// ----------------------------------------------------------------------

namespace art {

  class ProcessDesc;

  class EventProcessor : public IEventProcessor {
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

  private:
    void configureServices_(fhicl::ParameterSet const & pset);
    void initSchedules_(fhicl::ParameterSet const & pset);

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

    std::shared_ptr<ActivityRegistry>             actReg_;
    MFStatusUpdater                               mfStatusUpdater_;
    WorkerRegistry                                wreg_;
    MasterProductRegistry                         preg_;
    ServiceToken                                  serviceToken_;
    std::shared_ptr<InputSource>                  input_;
    tbb::task_scheduler_init tbbManager_;
    std::unique_ptr<Schedule>                       schedule_;
    ActionTable                                   act_table_;

    std::shared_ptr<FileBlock>                    fb_;

    std::unique_ptr<statemachine::Machine>          machine_;
    PrincipalCache                                principalCache_;
    std::unique_ptr<EventPrincipal>                 sm_evp_;
    bool                                          shouldWeStop_;
    bool                                          stateMachineWasInErrorState_;
    std::string                                   fileMode_;
    bool                                          handleEmptyRuns_;
    bool                                          handleEmptySubRuns_;
    std::string                                   exceptionMessageFiles_;
    std::string                                   exceptionMessageRuns_;
    std::string                                   exceptionMessageSubRuns_;
    bool                                          alreadyHandlingException_;
  };  // EventProcessor

  //--------------------------------------------------------------------

}  // art

// ======================================================================

#endif /* art_Framework_Core_EventProcessor_h */

// Local Variables:
// mode: c++
// End:
