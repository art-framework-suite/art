#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art
{

  EDAnalyzer::EDAnalyzer(fhicl::ParameterSet const& pset)
    : EventObserverBase{pset}
  {}

  bool
  EDAnalyzer::doEvent(EventPrincipal const& ep,
                      CPC_exempt_ptr cpc,
                      CountingStatistics& counts)
  {
    detail::CPCSentry sentry{current_context_, cpc};
    detail::PVSentry pvSentry{cachedProducts()};
    Event const e{ep, moduleDescription_, this};
    if (wantAllEvents() || wantEvent(e)) {
      // Run is incremented before analyze(e); to properly count
      // whenever an exception is thrown in the user's module.
      counts.increment<stats::Run>();
      analyze(e);
      counts.increment<stats::Passed>();
    }
    return true;
  }

  void
  EDAnalyzer::doBeginJob()
  {
    auto const& mainID = moduleDescription_.mainParameterSetID();
    auto const& scheduler_pset = fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>("services.scheduler");
    prepareForJob(scheduler_pset);
    beginJob();
  }

  void
  EDAnalyzer::doEndJob() {
    endJob();
    showMissingConsumes();
  }

  bool
  EDAnalyzer::doBeginRun(RunPrincipal const& rp,
                         CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run const r {rp, moduleDescription_, this};
    beginRun(r);
    return true;
  }

  bool
  EDAnalyzer::doEndRun(RunPrincipal const& rp,
                       CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run const r {rp, moduleDescription_, this};
    endRun(r);
    return true;
  }

  bool
  EDAnalyzer::doBeginSubRun(SubRunPrincipal const& srp,
                            CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun const sr {srp, moduleDescription_, this};
    beginSubRun(sr);
    return true;
  }

  bool
  EDAnalyzer::doEndSubRun(SubRunPrincipal const& srp,
                          CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun const sr {srp, moduleDescription_, this};
    endSubRun(sr);
    return true;
  }

  void
  EDAnalyzer::doRespondToOpenInputFile(FileBlock const& fb) {
    respondToOpenInputFile(fb);
  }

  void
  EDAnalyzer::doRespondToCloseInputFile(FileBlock const& fb) {
    respondToCloseInputFile(fb);
  }

  void
  EDAnalyzer::doRespondToOpenOutputFiles(FileBlock const& fb) {
    respondToOpenOutputFiles(fb);
  }

  void
  EDAnalyzer::doRespondToCloseOutputFiles(FileBlock const& fb) {
    respondToCloseOutputFiles(fb);
  }

  CurrentProcessingContext const*
  EDAnalyzer::currentContext() const {
    return current_context_.get();
  }

}  // art
