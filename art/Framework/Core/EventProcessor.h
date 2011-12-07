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
#include "art/Framework/Services/Registry/ServiceLegacy.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "boost/noncopyable.hpp"
#include "boost/thread/condition.hpp"
#include "boost/thread/thread.hpp"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <string>
#include <vector>

namespace statemachine {
  class Machine;
}

// ----------------------------------------------------------------------

namespace art {

  class ProcessDesc;

  class EventProcessor : public IEventProcessor, private boost::noncopyable
  {
  public:

    // The input string 'config' contains the entire contents of a  configuration file.
    // Also allows the attachement of pre-existing services specified  by 'token', and
    // the specification of services by name only (defaultServices and forcedServices).
    // 'defaultServices' are overridden by 'config'.
    // 'forcedServices' cause an exception if the same service is specified in 'config'.

    EventProcessor(fhicl::ParameterSet const& pset);
    ~EventProcessor();

    /**This should be called before the first call to 'run'.
       */
    void beginJob();

    /**This should be called before the EventProcessor is destroyed
       throws if any module's endJob throws an exception.
       */
    void endJob();

    /// signal is emitted after the Event has been created by the
    /// InputSource but before any modules have seen the Event
    ActivityRegistry::PreProcessEvent &
    preProcessEventSignal() {return preProcessEventSignal_;}

    /// signal is emitted after all modules have finished processing
    /// the Event
    ActivityRegistry::PostProcessEvent &
    postProcessEventSignal() {return postProcessEventSignal_;}

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
    virtual StatusCode runToCompletion();

    // The following functions are used by the code implementing our
    // boost statemachine

    virtual void readFile();
    virtual void closeInputFile();
    virtual void openOutputFiles();
    virtual void closeOutputFiles();

    virtual void respondToOpenInputFile();
    virtual void respondToCloseInputFile();
    virtual void respondToOpenOutputFiles();
    virtual void respondToCloseOutputFiles();

    virtual void startingNewLoop();
    virtual bool endOfLoop();
    virtual void rewindInput();
    virtual void prepareForNextLoop();
    virtual void writeSubRunCache();
    virtual void writeRunCache();
    virtual bool shouldWeCloseOutput() const;

    virtual void doErrorStuff();

    virtual void beginRun(int run);
    virtual void endRun(int run);

    virtual void beginSubRun(int run, int subRun);
    virtual void endSubRun(int run, int subRun);

    virtual int readAndCacheRun();
    virtual int readAndCacheSubRun();
    virtual void writeRun(int run);
    virtual void deleteRunFromCache(int run);
    virtual void writeSubRun(int run, int subRun);
    virtual void deleteSubRunFromCache(int run, int subRun);

    virtual void readEvent();
    virtual void processEvent();
    virtual bool shouldWeStop() const;

    virtual void setExceptionMessageFiles(std::string& message);
    virtual void setExceptionMessageRuns(std::string& message);
    virtual void setExceptionMessageSubRuns(std::string& message);
    virtual bool alreadyHandlingException() const;

  private:
    //------------------------------------------------------------------
    //
    // Now private functions.
    
    ServiceToken getToken();
    
    // init() is used by only by constructors
    void init(std::shared_ptr<art::ProcessDesc> & processDesc,
              ServiceToken const& token,
              ServiceLegacy);
    StatusCode runCommon(int numberOfEventsToProcess);
    void terminateMachine();
    void terminateAbnormally();
    void procOneEvent(EventPrincipal *pep);
    void connectSigs(EventProcessor * ep);
    void setupSignal();

    //------------------------------------------------------------------
    //
    // Data members below.
    // Are all these data members really needed? Some of them are used
    // only during construction, and never again. If they aren't
    // really needed, we should remove them.

    ActivityRegistry::PreProcessEvent             preProcessEventSignal_;
    ActivityRegistry::PostProcessEvent            postProcessEventSignal_;
    std::shared_ptr<ActivityRegistry>           actReg_;
    MFStatusUpdater                               mfStatusUpdater_;
    WorkerRegistry                                wreg_;
    MasterProductRegistry                         preg_;
    ServiceToken                                  serviceToken_;
    std::shared_ptr<InputSource>                input_;
    std::auto_ptr<Schedule>                       schedule_;
    ActionTable                                   act_table_;

    std::shared_ptr<boost::thread>              event_loop_;

    boost::mutex                                  state_lock_;
    boost::mutex                                  stop_lock_;
    boost::condition                              stopper_;
    boost::condition                              starter_;
    volatile int                                  stop_count_;
    volatile Status                               last_rc_;
    std::string                                   last_error_text_;
    volatile bool                                 id_set_;
    volatile pthread_t                            event_loop_id_;
    int                                           my_sig_num_;
    std::shared_ptr<FileBlock>                  fb_;

    std::auto_ptr<statemachine::Machine>          machine_;
    PrincipalCache                                principalCache_;
    std::auto_ptr<EventPrincipal>                 sm_evp_;
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
