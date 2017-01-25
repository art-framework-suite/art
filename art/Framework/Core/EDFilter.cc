#include "art/Framework/Core/EDFilter.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art
{

  bool
  EDFilter::doEvent(EventPrincipal& ep,
                    CPC_exempt_ptr cpc,
                    CountingStatistics& counts) {
    detail::CPCSentry sentry {current_context_, cpc};
    Event e {ep, moduleDescription_};
    bool const rc = filter(e);
    counts.increment<stats::Run>();
    e.commit_(ep, checkPutProducts_, expectedProducts());
    counts.update(rc);
    return rc;
  }

  void
  EDFilter::doBeginJob() {
    // 'checkPutProducts_' cannot be set during the c'tor
    // initialization list since 'moduleDescription_' is empty there.
    checkPutProducts_ = detail::get_failureToPut_flag( moduleDescription_ );
    beginJob();
  }

  void EDFilter::doEndJob() {
    endJob();
  }

  void
  EDFilter::reconfigure(fhicl::ParameterSet const&) {
    throw art::Exception(errors::UnimplementedFeature)
      << "Modules of type "
      << cet::demangle_symbol(typeid(*this).name())
      << " are not reconfigurable.\n";
  }

  bool
  EDFilter::doBeginRun(RunPrincipal & rp,
                       CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {rp, moduleDescription_, RangeSet::forRun(rp.id())};
    bool const rc = beginRun(r);
    r.commit_(rp);
    return rc;
  }

  bool
  EDFilter::doEndRun(RunPrincipal & rp,
                     CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {rp, moduleDescription_, rp.seenRanges()};
    bool const rc = endRun(r);
    r.commit_(rp);
    return rc;
  }

  bool
  EDFilter::doBeginSubRun(SubRunPrincipal & srp,
                          CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {srp, moduleDescription_, RangeSet::forSubRun(srp.id())};
    bool const rc = beginSubRun(sr);
    sr.commit_(srp);
    return rc;
  }

  bool
  EDFilter::doEndSubRun(SubRunPrincipal & srp,
                        CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {srp, moduleDescription_, srp.seenRanges()};
    bool const rc = endSubRun(sr);
    sr.commit_(srp);
    return rc;


  }

  void
  EDFilter::doRespondToOpenInputFile(FileBlock const& fb) {
    respondToOpenInputFile(fb);
  }

  void
  EDFilter::doRespondToCloseInputFile(FileBlock const& fb) {
    respondToCloseInputFile(fb);
  }

  void
  EDFilter::doRespondToOpenOutputFiles(FileBlock const& fb) {
    respondToOpenOutputFiles(fb);
  }

  void
  EDFilter::doRespondToCloseOutputFiles(FileBlock const& fb) {
    respondToCloseOutputFiles(fb);
  }

  CurrentProcessingContext const*
  EDFilter::currentContext() const {
    return current_context_.get();
  }

}  // art
