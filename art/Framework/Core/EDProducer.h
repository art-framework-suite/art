#ifndef FWCore_Framework_EDProducer_h
#define FWCore_Framework_EDProducer_h

// ======================================================================
//
// EDProducer - The base class of "modules" whose main purpose is to
//              insert new EDProducts into an Event.
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include <ostream>
#include <string>

// ----------------------------------------------------------------------

namespace art
{

  class EDProducer
    : public ProducerBase
    , public EngineCreator
  {
  public:
    template <typename T> friend class WorkerT;
    typedef EDProducer ModuleType;
    typedef WorkerT<EDProducer> WorkerType;

    EDProducer ();
    virtual ~EDProducer();

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('produce').
    CurrentProcessingContext const* currentContext() const;

  private:
    bool doEvent(EventPrincipal& ep,
                   CurrentProcessingContext const* cpcp);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doEndRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doBeginSubRun(SubRunPrincipal & srp,
                   CurrentProcessingContext const* cpc);
    bool doEndSubRun(SubRunPrincipal & srp,
                   CurrentProcessingContext const* cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);
    void registerAnyProducts(boost::shared_ptr<EDProducer>& module, ProductRegistry *reg) {
      registerProducts(module, reg, moduleDescription_);
    }

    std::string workerType() const {return "WorkerT<EDProducer>";}

    virtual void produce(Event &) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual void reconfigure(std::istream&,
			     std::ostream& output,
			     ParameterSet const&)
    { output << "This module is not reconfigurable." << std::endl;}

    virtual void beginRun(Run &){}
    virtual void endRun(Run &){}
    virtual void beginSubRun(SubRun &){}
    virtual void endSubRun(SubRun &){}
    virtual void respondToOpenInputFile(FileBlock const& fb) {}
    virtual void respondToCloseInputFile(FileBlock const& fb) {}
    virtual void respondToOpenOutputFiles(FileBlock const& fb) {}
    virtual void respondToCloseOutputFiles(FileBlock const& fb) {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }
    ModuleDescription moduleDescription_;
    CurrentProcessingContext const* current_context_;
  };  // EDProducer

}  // art

// ======================================================================

#endif
