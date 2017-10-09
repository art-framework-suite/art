#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

//
// The base class of all "modules" used to control the flow of
// processing in a trigger path.  Filters can also insert products
// into the event.  These products should be informational products
// about the filter decision.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
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
  class EDFilter : public ProducerBase {

    // Allow the WorkerT<T> ctor to call setModuleDescription() and
    // workerType().
    template <typename T>
    friend class WorkerT;

  public: // TYPES
    // The module macros need these two.
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;

    static constexpr bool Pass{true};
    static constexpr bool Fail{false};

  public: // CONFIGURATION
    template <typename UserConfig>
    using Table = ProducerBase::Table<UserConfig>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~EDFilter();

    EDFilter();

    EDFilter(EDFilter const&) = delete;

    EDFilter(EDFilter&&) = delete;

    EDFilter& operator=(EDFilter const&) = delete;

    EDFilter& operator=(EDFilter&&) = delete;

  private: // MEMBER FUNCTIONS
    std::string workerType() const;

  private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
           // EndPathExecutor
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
    // Not called by framework
    virtual void reconfigure(fhicl::ParameterSet const&);

    virtual void beginJob();

    virtual void endJob();

    virtual void respondToOpenInputFile(FileBlock const&);

    virtual void respondToCloseInputFile(FileBlock const&);

    virtual void respondToOpenOutputFiles(FileBlock const&);

    virtual void respondToCloseOutputFiles(FileBlock const&);

    virtual bool beginRun(Run&);

    virtual bool endRun(Run&);

    virtual bool beginSubRun(SubRun&);

    virtual bool endSubRun(SubRun&);

    virtual bool filter(Event&) = 0;

    // virtual
    // bool
    // filter_in_stream(Event&, int streamIndex);

  protected: // MEMBER DATA -- For derived classes
    bool checkPutProducts_{true};
  };

  namespace one {

    // <<pure virtual abstract base>>
    class EDFilter : public art::EDFilter {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDFilter();

      EDFilter();

      EDFilter(EDFilter const&) = delete;

      EDFilter(EDFilter&&) = delete;

      EDFilter& operator=(EDFilter const&) = delete;

      EDFilter& operator=(EDFilter&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace one

  namespace stream {

    // <<pure virtual abstract base>>
    class EDFilter : public art::EDFilter {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDFilter();

      EDFilter();

      EDFilter(EDFilter const&) = delete;

      EDFilter(EDFilter&&) = delete;

      EDFilter& operator=(EDFilter const&) = delete;

      EDFilter& operator=(EDFilter&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace stream

  namespace global {

    // <<pure virtual abstract base>>
    class EDFilter : public art::EDFilter {

      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~EDFilter();

      EDFilter();

      EDFilter(EDFilter const&) = delete;

      EDFilter(EDFilter&&) = delete;

      EDFilter& operator=(EDFilter const&) = delete;

      EDFilter& operator=(EDFilter&&) = delete;

    protected
      : // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and
        // EndPathExecutor.
      void doBeginJob() override;
    };

  } // namespace global

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
