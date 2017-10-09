#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

//
// The base class of "modules" whose main purpose is to
// insert new EDProducts into an Event.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace art {

  // <<pure virtual abstract base>>
  class EDProducer : public ProducerBase {

    // Allow the WorkerT<T> ctor to call setModuleDescription() and
    // workerType().
    template <typename T>
    friend class WorkerT;

  public: // TYPES
    // The module macros need these two.
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;

  public: // CONFIGURATION
    template <typename UserConfig>
    using Table = ProducerBase::Table<UserConfig>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~EDProducer();

    EDProducer();

    EDProducer(EDProducer const&) = delete;

    EDProducer(EDProducer&&) = delete;

    EDProducer& operator=(EDProducer const&) = delete;

    EDProducer& operator=(EDProducer&&) = delete;

  protected: // MEMBER FUNCTIONS
    std::string workerType() const;

  protected
    : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
      // EndPathExecutor.
    virtual void doBeginJob();

    void doEndJob();

    void doRespondToOpenInputFile(FileBlock const& fb);

    void doRespondToCloseInputFile(FileBlock const& fb);

    void doRespondToOpenOutputFiles(FileBlock const& fb);

    void doRespondToCloseOutputFiles(FileBlock const& fb);

    bool doBeginRun(RunPrincipal& rp,
                    cet::exempt_ptr<CurrentProcessingContext const> cpc);

    bool doEndRun(RunPrincipal& rp,
                  cet::exempt_ptr<CurrentProcessingContext const> cpc);

    bool doBeginSubRun(SubRunPrincipal& srp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc);

    bool doEndSubRun(SubRunPrincipal& srp,
                     cet::exempt_ptr<CurrentProcessingContext const> cpc);

    bool doEvent(EventPrincipal& ep,
                 int streamIndex,
                 CurrentProcessingContext const* cpc,
                 std::atomic<std::size_t>& counts_run,
                 std::atomic<std::size_t>& counts_passed,
                 std::atomic<std::size_t>& counts_failed);

  protected
    : // MEMBER FUNCTIONS -- Implementation API, intended to be provided by
      // derived classes.
    // Not called by framework.
    virtual void reconfigure(fhicl::ParameterSet const&);

    virtual void beginJob();

    virtual void endJob();

    virtual void respondToOpenInputFile(FileBlock const&);

    virtual void respondToCloseInputFile(FileBlock const&);

    virtual void respondToOpenOutputFiles(FileBlock const&);

    void virtual respondToCloseOutputFiles(FileBlock const&);

    virtual void beginRun(Run&);

    virtual void endRun(Run&);

    virtual void beginSubRun(SubRun&);

    virtual void endSubRun(SubRun&);

    // We make this pure virtual because a user module that does
    // not provide one would only have side-effects, and we do
    // not want people doing that.
    virtual void produce(Event&) = 0;

    // virtual
    // void
    // produce_in_stream(Event&, int streamIndex);

  protected: // MEMBER DATA -- For derived classes
    bool checkPutProducts_{true};
  };

  namespace one {

    class EDProducer : public art::EDProducer {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDProducer();

      EDProducer();

      EDProducer(EDProducer const&) = delete;

      EDProducer(EDProducer&&) = delete;

      EDProducer& operator=(EDProducer const&) = delete;

      EDProducer& operator=(EDProducer&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace one

  namespace stream {

    class EDProducer : public art::EDProducer {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDProducer();

      EDProducer();

      EDProducer(EDProducer const&) = delete;

      EDProducer(EDProducer&&) = delete;

      EDProducer& operator=(EDProducer const&) = delete;

      EDProducer& operator=(EDProducer&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace stream

  namespace global {

    class EDProducer : public art::EDProducer {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDProducer();

      EDProducer();

      EDProducer(EDProducer const&) = delete;

      EDProducer(EDProducer&&) = delete;

      EDProducer& operator=(EDProducer const&) = delete;

      EDProducer& operator=(EDProducer&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace global

} // namespace art

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
