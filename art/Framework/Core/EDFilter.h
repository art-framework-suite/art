#ifndef FWCore_Framework_EDFilter_h
#define FWCore_Framework_EDFilter_h

/*----------------------------------------------------------------------

EDFilter: The base class of all "modules" used to control the flow of
processing in a processing path.
Filters can also insert products into the event.
These products should be informational products about the filter decision.

----------------------------------------------------------------------*/


#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/ParameterSet/ParameterSetfwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include <string>


namespace art
{

  class EDFilter
    : public ProducerBase
    , public EngineCreator
  {
  public:
    template <typename T> friend class WorkerT;
    typedef EDFilter ModuleType;
    typedef WorkerT<EDFilter> WorkerType;

    EDFilter()
      : ProducerBase()
      , EngineCreator()
      , moduleDescription_()
      , current_context_(0)
    { }
    virtual ~EDFilter();

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('filter').
    CurrentProcessingContext const* currentContext() const;

  private:
    bool doEvent(EventPrincipal& ep,
                  CurrentProcessingContext const* cpc);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doEndRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doBeginSubRun(SubRunPrincipal & lbp,
                   CurrentProcessingContext const* cpc);
    bool doEndSubRun(SubRunPrincipal & lbp,
                   CurrentProcessingContext const* cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);
    void registerAnyProducts(boost::shared_ptr<EDFilter>&module, ProductRegistry *reg) {
      registerProducts(module, reg, moduleDescription_);
    }

    std::string workerType() const {return "WorkerT<EDFilter>";}

    virtual bool filter(Event&) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual bool beginRun(Run &){return true;}
    virtual bool endRun(Run &){return true;}
    virtual bool beginSubRun(SubRun &){return true;}
    virtual bool endSubRun(SubRun &){return true;}
    virtual void respondToOpenInputFile(FileBlock const& fb) {}
    virtual void respondToCloseInputFile(FileBlock const& fb) {}
    virtual void respondToOpenOutputFiles(FileBlock const& fb) {}
    virtual void respondToCloseOutputFiles(FileBlock const& fb) {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }
    ModuleDescription moduleDescription_;
    CurrentProcessingContext const* current_context_;
  };

}  // namespace art

#endif  // FWCore_Framework_EDFilter_h
