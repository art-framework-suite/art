#include "art/Framework/Core/EDFilter.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art
{

  EDFilter::EDFilter()
    : ProducerBase()
    , EngineCreator()
    , moduleDescription_()
    , current_context_(0)
    , checkPutProducts_{true}
  {}

  bool
  EDFilter::doEvent(EventPrincipal& ep,
                    CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    Event e(ep, moduleDescription_);
    rc = this->filter(e);
    e.commit_(checkPutProducts_, expectedProducts());
    return rc;
  }

  void
  EDFilter::doBeginJob() {
    // 'checkPutProducts_' cannot be set during the c'tor
    // initialization list since 'moduleDescription_' is empty there.
    checkPutProducts_ = detail::get_failureToPut_flag( moduleDescription_ );
    this->beginJob();
  }

  void EDFilter::doEndJob() {
    this->endJob();
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
                       CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    Run r(rp, moduleDescription_);
    rc = this->beginRun(r);
    r.commit_();
    return rc;
  }

  bool
  EDFilter::doEndRun(RunPrincipal & rp,
                     CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    Run r(rp, moduleDescription_);
    rc = this->endRun(r);
    r.commit_();
    return rc;
  }

  bool
  EDFilter::doBeginSubRun(SubRunPrincipal & srp,
                          CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    SubRun sr(srp, moduleDescription_);
    rc = this->beginSubRun(sr);
    sr.commit_();
    return rc;
  }

  bool
  EDFilter::doEndSubRun(SubRunPrincipal & srp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    SubRun sr(srp, moduleDescription_);
    rc = this->endSubRun(sr);
    sr.commit_();
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
    return current_context_;
  }

}  // art
