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
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace art {

  class EDFilter : public ProducerBase {
    friend class WorkerT<EDFilter>;

  public: // TYPES
    // The module macros need these two.
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;
    static constexpr bool Pass{true};
    static constexpr bool Fail{false};
    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::legacy;
    }

  public: // CONFIGURATION
    template <typename UserConfig>
    using Table = ProducerBase::Table<UserConfig>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~EDFilter() noexcept;
    EDFilter();
    EDFilter(EDFilter const&) = delete;
    EDFilter(EDFilter&&) = delete;
    EDFilter& operator=(EDFilter const&) = delete;
    EDFilter& operator=(EDFilter&&) = delete;

    // FIXME: Change this to private when you're ready

    virtual void doBeginJob();
    std::string workerType() const;

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
                 ScheduleID const,
                 CurrentProcessingContext const* cpc,
                 std::atomic<std::size_t>& counts_run,
                 std::atomic<std::size_t>& counts_passed,
                 std::atomic<std::size_t>& counts_failed);

  protected:
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

  private:
    bool checkPutProducts_{true};
  };

  namespace shared {
    class Filter : public art::EDFilter {
      friend class WorkerT<Filter>;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      using ModuleType = Filter;
      using WorkerType = WorkerT<Filter>;

      static constexpr ModuleThreadingType
      moduleThreadingType()
      {
        return ModuleThreadingType::shared;
      }
      virtual ~Filter() noexcept;
      Filter();
      Filter(Filter const&) = delete;
      Filter(Filter&&) = delete;
      Filter& operator=(Filter const&) = delete;
      Filter& operator=(Filter&&) = delete;

      void doBeginJob() override;

    private:
      bool checkPutProducts_{true};
    };

  } // namespace shared

  namespace replicated {
    class Filter : public art::EDFilter {
      friend class WorkerT<Filter>;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      using ModuleType = Filter;
      using WorkerType = WorkerT<Filter>;

      static constexpr ModuleThreadingType
      moduleThreadingType()
      {
        return ModuleThreadingType::replicated;
      }

      virtual ~Filter() noexcept;
      Filter();
      Filter(Filter const&) = delete;
      Filter(Filter&&) = delete;
      Filter& operator=(Filter const&) = delete;
      Filter& operator=(Filter&&) = delete;

      void doBeginJob() override;

    private:
      bool checkPutProducts_{true};
    };
  } // namespace replicated
} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
