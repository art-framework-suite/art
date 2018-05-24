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
#include "fhiclcpp/ParameterSet.h"

namespace art {

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
    Run r{rp, mc_, RangeSet::forRun(rp.runID())};
    postReadRun(r);
    r.movePutProductsToPrincipal(rp);
  }

  void
  ProducingService::doPostReadSubRun(SubRunPrincipal& srp)
  {
    SubRun sr{srp, mc_, RangeSet::forSubRun(srp.subRunID())};
    postReadSubRun(sr);
    sr.movePutProductsToPrincipal(srp);
  }

  void
  ProducingService::doPostReadEvent(EventPrincipal& ep)
  {
    Event e{ep, mc_};
    postReadEvent(e);
    e.movePutProductsToPrincipal(ep, true, &expectedProducts<InEvent>());
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
