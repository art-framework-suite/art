#include "art/Framework/Core/EDProducer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {

  bool
  EDProducer::doEvent(EventPrincipal& ep,
                      CPC_exempt_ptr cpc,
                      CountingStatistics& counts)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    Event e{ep, moduleDescription_, this};
    counts.increment<stats::Run>();
    produce(e);
    e.commit(ep, checkPutProducts_, expectedProducts<InEvent>());
    counts.increment<stats::Passed>();
    return true;
  }

  void
  EDProducer::doBeginJob()
  {
    // 'checkPutProducts_' cannot be set during the c'tor
    // initialization list since 'moduleDescription_' is empty there.
    auto const& mainID = moduleDescription_.mainParameterSetID();
    auto const& scheduler_pset =
      fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
        "services.scheduler");
    auto const& module_pset =
      fhicl::ParameterSetRegistry::get(moduleDescription_.parameterSetID());
    checkPutProducts_ =
      detail::get_failureToPut_flag(scheduler_pset, module_pset);
    prepareForJob(scheduler_pset);
    beginJob();
  }

  void
  EDProducer::doEndJob()
  {
    endJob();
    showMissingConsumes();
  }

  bool
  EDProducer::doBeginRun(RunPrincipal& rp, CPC_exempt_ptr cpc)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    Run r{rp, moduleDescription_, this, RangeSet::forRun(rp.id())};
    beginRun(r);
    r.commit(rp);
    return true;
  }

  bool
  EDProducer::doEndRun(RunPrincipal& rp, CPC_exempt_ptr cpc)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    Run r{rp, moduleDescription_, this, rp.seenRanges()};
    endRun(r);
    r.commit(rp);
    return true;
  }

  bool
  EDProducer::doBeginSubRun(SubRunPrincipal& srp, CPC_exempt_ptr cpc)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    SubRun sr{srp, moduleDescription_, this, RangeSet::forSubRun(srp.id())};
    beginSubRun(sr);
    sr.commit(srp);
    return true;
  }

  bool
  EDProducer::doEndSubRun(SubRunPrincipal& srp, CPC_exempt_ptr cpc)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    SubRun sr{srp, moduleDescription_, this, srp.seenRanges()};
    endSubRun(sr);
    sr.commit(srp);
    return true;
  }

  void
  EDProducer::doRespondToOpenInputFile(FileBlock const& fb)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDProducer::doRespondToCloseInputFile(FileBlock const& fb)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDProducer::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDProducer::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    respondToCloseOutputFiles(fb);
  }

  CurrentProcessingContext const*
  EDProducer::currentContext() const
  {
    return current_context_.get();
  }

} // art
