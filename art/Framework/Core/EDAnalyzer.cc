#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/demangle.h"

namespace art
{

  bool
  EDAnalyzer::doEvent(EventPrincipal const& ep,
                      CPC_exempt_ptr cpc,
                      CountingStatistics& counts)
  {
    detail::CPCSentry sentry {current_context_, cpc};
    detail::PVSentry pvSentry {cachedProducts()};
    Event e {const_cast<EventPrincipal&>(ep), moduleDescription_};
    if (wantAllEvents() || wantEvent(e)) {
      analyze(e);
      counts.increment<stats::Run, stats::Passed>();
    }
    return true;
  }

  void
  EDAnalyzer::doBeginJob() {
    beginJob();
  }

  void
  EDAnalyzer::doEndJob() {
    endJob();
  }

  void
  EDAnalyzer::reconfigure(fhicl::ParameterSet const&) {
    throw art::Exception(errors::UnimplementedFeature)
      << "Modules of type "
      << cet::demangle_symbol(typeid(*this).name())
      << " are not reconfigurable.\n";
  }

  bool
  EDAnalyzer::doBeginRun(RunPrincipal const& rp,
                         CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {const_cast<RunPrincipal &>(rp), moduleDescription_};
    beginRun(r);
    return true;
  }

  bool
  EDAnalyzer::doEndRun(RunPrincipal const& rp,
                       CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {const_cast<RunPrincipal &>(rp), moduleDescription_};
    endRun(r);
    return true;
  }

  bool
  EDAnalyzer::doBeginSubRun(SubRunPrincipal const& srp,
                            CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {const_cast<SubRunPrincipal &>(srp), moduleDescription_};
    beginSubRun(sr);
    return true;
  }

  bool
  EDAnalyzer::doEndSubRun(SubRunPrincipal const& srp,
                          CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {const_cast<SubRunPrincipal &>(srp), moduleDescription_};
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
