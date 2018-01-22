#include "art/Framework/Core/ProducingService.h"
#include "art/Framework/Core/ProducingServiceSignals.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

art::ProducingService::~ProducingService() noexcept = default;

void
art::ProducingService::registerCallbacks(ProducingServiceSignals& signals)
{
  signals.sPostReadRun.watch(this, &ProducingService::doPostReadRun);
  signals.sPostReadSubRun.watch(this, &ProducingService::doPostReadSubRun);
  signals.sPostReadEvent.watch(this, &ProducingService::doPostReadEvent);
}

void
art::ProducingService::doPostReadRun(RunPrincipal& rp)
{
  Run r{rp, md_, expectedProducts<InRun>(), RangeSet::forRun(rp.runID())};
  postReadRun(r);
  r.commit(rp, true);
}

void
art::ProducingService::doPostReadSubRun(SubRunPrincipal& srp)
{
  SubRun sr{srp,
            md_,
            expectedProducts<InSubRun>(),
            RangeSet::forSubRun(srp.subRunID())};
  postReadSubRun(sr);
  sr.commit(srp, true);
}

void
art::ProducingService::doPostReadEvent(EventPrincipal& ep)
{
  Event e{ep, md_, expectedProducts<InEvent>()};
  postReadEvent(e);
  e.commit(ep, true);
}

void
art::ProducingService::postReadRun(Run&)
{}

void
art::ProducingService::postReadSubRun(SubRun&)
{}

void
art::ProducingService::postReadEvent(Event&)
{}

void
art::ProducingService::setModuleDescription(ModuleDescription const& md)
{
  md_ = md;
}
