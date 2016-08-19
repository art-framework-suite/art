#include "art/Framework/Core/EDProducer.h"

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
  EDProducer::doEvent(EventPrincipal& ep,
                      CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Event e {ep, moduleDescription_};
    produce(e);
    e.commit_(checkPutProducts_, expectedProducts());
    return true;
  }

  void
  EDProducer::doBeginJob() {
    // 'checkPutProducts_' cannot be set during the c'tor
    // initialization list since 'moduleDescription_' is empty there.
    checkPutProducts_ = detail::get_failureToPut_flag( moduleDescription_ );
    beginJob();
  }

  void
  EDProducer::doEndJob() {
    endJob();
  }

  void
  EDProducer::reconfigure(fhicl::ParameterSet const&) {
    throw art::Exception(errors::UnimplementedFeature)
      << "Modules of type "
      << cet::demangle_symbol(typeid(*this).name())
      << " are not reconfigurable.\n";
  }

  bool
  EDProducer::doBeginRun(RunPrincipal & rp,
                         CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {rp, moduleDescription_, RangeSet::forRun(rp.id())};
    beginRun(r);
    r.commit_();
    return true;
  }

  bool
  EDProducer::doEndRun(RunPrincipal & rp,
                       CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    Run r {rp, moduleDescription_, rp.seenRanges()};
    endRun(r);
    r.commit_();
    return true;
  }

  bool
  EDProducer::doBeginSubRun(SubRunPrincipal & srp,
                            CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {srp, moduleDescription_, RangeSet::forSubRun(srp.id())};
    beginSubRun(sr);
    sr.commit_();
    return true;
  }

  bool
  EDProducer::doEndSubRun(SubRunPrincipal & srp,
                          CPC_exempt_ptr cpc) {
    detail::CPCSentry sentry {current_context_, cpc};
    SubRun sr {srp, moduleDescription_, srp.seenRanges()};
    endSubRun(sr);
    sr.commit_();
    return true;
  }

  void
  EDProducer::doRespondToOpenInputFile(FileBlock const& fb) {
    respondToOpenInputFile(fb);
  }

  void
  EDProducer::doRespondToCloseInputFile(FileBlock const& fb) {
    respondToCloseInputFile(fb);
  }

  void
  EDProducer::doRespondToOpenOutputFiles(FileBlock const& fb) {
    respondToOpenOutputFiles(fb);
  }

  void
  EDProducer::doRespondToCloseOutputFiles(FileBlock const& fb) {
    respondToCloseOutputFiles(fb);
  }

  CurrentProcessingContext const*
  EDProducer::currentContext() const {
    return current_context_.get();
  }

}  // art
