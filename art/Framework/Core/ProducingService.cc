#include "art/Framework/Core/ProducingService.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProducingServiceSignals.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {

  ProducingService::ProducingService()
    : ProductRegistryHelper{product_creation_mode::produces}
  {}
  ProducingService::~ProducingService() noexcept = default;

  void
  ProducingService::setModuleDescription(ModuleDescription const& md)
  {
    // We choose the one-argument constructor since the path
    // information is irrelevant when the doPostRead* functions are
    // invoked.
    mc_ = ModuleContext{md};
  }

  void
  ProducingService::registerCallbacks(ProducingServiceSignals& cbReg)
  {
    cbReg.sPostReadRun.watch(this, &ProducingService::doPostReadRun);
    cbReg.sPostReadSubRun.watch(this, &ProducingService::doPostReadSubRun);
    cbReg.sPostReadEvent.watch(this, &ProducingService::doPostReadEvent);
  }

  void
  ProducingService::doPostReadRun(RunPrincipal& rp)
  {
    auto r = Run::make(rp, mc_, RangeSet::forRun(rp.runID()));
    postReadRun(r);
    r.commitProducts();
  }

  void
  ProducingService::doPostReadSubRun(SubRunPrincipal& srp)
  {
    auto sr = SubRun::make(srp, mc_, RangeSet::forSubRun(srp.subRunID()));
    postReadSubRun(sr);
    sr.commitProducts();
  }

  void
  ProducingService::doPostReadEvent(EventPrincipal& ep)
  {
    auto e = Event::make(ep, mc_);
    postReadEvent(e);
    e.commitProducts(true, &expectedProducts<InEvent>());
  }

  void
  ProducingService::postReadRun(Run&)
  {}

  void
  ProducingService::postReadSubRun(SubRun&)
  {}

  void
  ProducingService::postReadEvent(Event&)
  {}

} // namespace art
