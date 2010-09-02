/*----------------------------------------------------------------------


----------------------------------------------------------------------*/

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/LuminosityBlock.h"
#include "art/Framework/Core/Run.h"

#include "art/ParameterSet/ParameterSetDescription.h"

namespace edm
{
  EDFilter::~EDFilter()
  { }

  bool
  EDFilter::doEvent(EventPrincipal& ep,
		     CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    Event e(ep, moduleDescription_);
    rc = this->filter(e);
    e.commit_();
    return rc;
  }

  void
  EDFilter::doBeginJob() {
    this->beginJob();
  }

  void EDFilter::doEndJob() {
    this->endJob();
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
  EDFilter::doBeginLuminosityBlock(LuminosityBlockPrincipal & lbp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    LuminosityBlock lb(lbp, moduleDescription_);
    rc = this->beginLuminosityBlock(lb);
    lb.commit_();
    return rc;
  }

  bool
  EDFilter::doEndLuminosityBlock(LuminosityBlockPrincipal & lbp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    bool rc = false;
    LuminosityBlock lb(lbp, moduleDescription_);
    rc = this->endLuminosityBlock(lb);
    lb.commit_();
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

  void
  EDFilter::fillDescription(edm::ParameterSetDescription& iDesc,
                            std::string const& moduleLabel) {
    iDesc.setUnknown();
  }

}
