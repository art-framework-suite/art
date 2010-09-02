/*----------------------------------------------------------------------


----------------------------------------------------------------------*/

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"

#include "art/ParameterSet/ParameterSetDescription.h"

namespace edm {
  EDProducer::EDProducer() :
      ProducerBase(),
      EngineCreator(),
      moduleDescription_(),
      current_context_(0) {}

  EDProducer::~EDProducer() { }

  bool
  EDProducer::doEvent(EventPrincipal& ep,
			     CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Event e(ep, moduleDescription_);
    this->produce(e);
    e.commit_();
    return true;
  }

  void
  EDProducer::doBeginJob() {
    this->beginJob();
  }

  void
  EDProducer::doEndJob() {
    this->endJob();
  }

  bool
  EDProducer::doBeginRun(RunPrincipal & rp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Run r(rp, moduleDescription_);
    this->beginRun(r);
    r.commit_();
    return true;
  }

  bool
  EDProducer::doEndRun(RunPrincipal & rp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Run r(rp, moduleDescription_);
    this->endRun(r);
    r.commit_();
    return true;
  }

  bool
  EDProducer::doBeginSubRun(SubRunPrincipal & lbp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun lb(lbp, moduleDescription_);
    this->beginSubRun(lb);
    lb.commit_();
    return true;
  }

  bool
  EDProducer::doEndSubRun(SubRunPrincipal & lbp,
			CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun lb(lbp, moduleDescription_);
    this->endSubRun(lb);
    lb.commit_();
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
    return current_context_;
  }

  void
  EDProducer::fillDescription(edm::ParameterSetDescription& iDesc,
                              std::string const& moduleLabel) {
    iDesc.setUnknown();
  }
}
