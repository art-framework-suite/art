#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h

// ======================================================================
//
// EDFilter - The base class of all "modules" used to control the flow of
// processing in a processing path.  Filters can also insert products
// into the event.  These products should be informational products about
// the filter decision.
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/ConsumesRecorder.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art
{

  class EDFilter : public ProducerBase,
                   public EngineCreator
  {
  public:
    static constexpr bool Pass{true};
    static constexpr bool Fail{false};

    template <typename T> friend class WorkerT;
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;

    virtual ~EDFilter() = default;

    template <typename PROD, BranchType B = InEvent>
    ProductID
    getProductID(std::string const& instanceName = {}) const;

    template <typename UserConfig>
    using Table = ProducerBase::Table<UserConfig>;

  protected:

    template <typename T, BranchType BT = InEvent>
    void consumes(InputTag const& it) { consumesRecorder_.consumes<T, BT>(it); }

    template <typename T, BranchType BT = InEvent>
    void consumesMany() { consumesRecorder_.consumesMany<T, BT>(); }

    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('filter').
    CurrentProcessingContext const* currentContext() const;

  private:

    using CPC_exempt_ptr = cet::exempt_ptr<CurrentProcessingContext const>;

    bool doEvent(EventPrincipal& ep, CPC_exempt_ptr cpc, CountingStatistics&);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal& rp, CPC_exempt_ptr cpc);
    bool doEndRun(RunPrincipal& rp, CPC_exempt_ptr cpc);
    bool doBeginSubRun(SubRunPrincipal& srp, CPC_exempt_ptr cpc);
    bool doEndSubRun(SubRunPrincipal& srp, CPC_exempt_ptr cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);

    std::string workerType() const {return "WorkerT<EDFilter>";}

    virtual bool filter(Event&) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual void reconfigure(fhicl::ParameterSet const&) {} // Not called by framework
    virtual bool beginRun(Run&){return true;}
    virtual bool endRun(Run&){return true;}
    virtual bool beginSubRun(SubRun&){return true;}
    virtual bool endSubRun(SubRun&){return true;}
    virtual void respondToOpenInputFile(FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}
    virtual void respondToOpenOutputFiles(FileBlock const&) {}
    virtual void respondToCloseOutputFiles(FileBlock const&) {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
      // Since the module description in the ConsumesRecorder class is
      // owned by pointer, we must give it the owned object of this
      // class--i.e. moduleDescription_, not md.
      consumesRecorder_.setModuleDescription(moduleDescription_);
    }

    ModuleDescription moduleDescription_{};
    ConsumesRecorder consumesRecorder_{};
    CPC_exempt_ptr current_context_{nullptr};
    bool checkPutProducts_{true};
  };  // EDFilter

  template <typename PROD, BranchType B>
  inline
  ProductID
  EDFilter::getProductID(std::string const& instanceName) const
  {
    return ProducerBase::getProductID<PROD, B>(moduleDescription_,
                                               instanceName);
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
