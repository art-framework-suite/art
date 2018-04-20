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
    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::LEGACY;
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

  private: // MEMBER FUNCTIONS
    std::string workerType() const;

  private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule,
           // and EndPathExecutor
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

  protected: // FIXME!!!!!
    bool checkPutProducts_{true};
  };

  namespace shared {
    class Filter : public art::EDFilter {
      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      static constexpr ModuleThreadingType
      moduleThreadingType()
      {
        return ModuleThreadingType::SHARED;
      }
      virtual ~Filter() noexcept;
      Filter();
      Filter(Filter const&) = delete;
      Filter(Filter&&) = delete;
      Filter& operator=(Filter const&) = delete;
      Filter& operator=(Filter&&) = delete;

    private:
      void doBeginJob() override;
    };

  } // namespace shared

  namespace replicated {
    class Filter : public art::EDFilter {
      // Allow the WorkerT<T> ctor to call setModuleDescription() and
      // workerType().
      template <typename T>
      friend class WorkerT;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      static constexpr ModuleThreadingType
      moduleThreadingType()
      {
        return ModuleThreadingType::REPLICATED;
      }
      virtual ~Filter() noexcept;
      Filter();
      Filter(Filter const&) = delete;
      Filter(Filter&&) = delete;
      Filter& operator=(Filter const&) = delete;
      Filter& operator=(Filter&&) = delete;

    private:
      void doBeginJob() override;
    };
  } // namespace replicated
} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
