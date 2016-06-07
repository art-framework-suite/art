#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h

// ======================================================================
//
// EDProducer - The base class of "modules" whose main purpose is to
//              insert new EDProducts into an Event.
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
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
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;

    virtual ~EDProducer() = default;

    template <typename PROD, BranchType B, typename TRANS>
    ProductID
    getProductID(TRANS const &translator,
                 std::string const& instanceName=std::string()) const;

    template <typename PROD, typename TRANS>
    ProductID
    getProductID(TRANS const &translator,
                 std::string const& instanceName=std::string()) const;

    template <typename UserConfig>
    using Table = ProducerBase::Table<UserConfig>;

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('produce').
    CurrentProcessingContext const* currentContext() const;

  private:

    using CPC_exempt_ptr = cet::exempt_ptr<CurrentProcessingContext const>;

    bool doEvent(EventPrincipal& ep, CPC_exempt_ptr cpcp);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal & rp, CPC_exempt_ptr cpc);
    bool doEndRun(RunPrincipal & rp, CPC_exempt_ptr cpc);
    bool doBeginSubRun(SubRunPrincipal & srp, CPC_exempt_ptr cpc);
    bool doEndSubRun(SubRunPrincipal & srp, CPC_exempt_ptr cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);
    void doRespondToOpenOutputFile();
    void doRespondToCloseOutputFile();

    std::string workerType() const {return "WorkerT<EDProducer>";}

    virtual void produce(Event &) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual void reconfigure(fhicl::ParameterSet const&);

    virtual void beginRun(Run &){}
    virtual void beginSubRun(SubRun &){}
    virtual void endRun(Run &){}
    virtual void endSubRun(SubRun &){}

    virtual void respondToOpenInputFile(FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}
    virtual void respondToOpenOutputFiles(FileBlock const&) {}
    virtual void respondToCloseOutputFiles(FileBlock const&) {}
    virtual void respondToOpenOutputFile() {}
    virtual void respondToCloseOutputFile() {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }

    ModuleDescription moduleDescription_ {};
    CPC_exempt_ptr current_context_ {nullptr};
    bool checkPutProducts_ {true};
  };  // EDProducer

  template <typename PROD, BranchType B, typename TRANS>
  inline
  ProductID
  EDProducer::getProductID(TRANS const &translator,
                           std::string const& instanceName) const {
    return ProducerBase::getProductID<PROD, B>(translator,
                                               moduleDescription_,
                                               instanceName);
  }

  template <typename PROD, typename TRANS>
  inline
  ProductID
  EDProducer::getProductID(TRANS const &translator,
                           std::string const& instanceName) const {
    return ProducerBase::getProductID<PROD, InEvent>(translator,
                                                     moduleDescription_,
                                                     instanceName);
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
