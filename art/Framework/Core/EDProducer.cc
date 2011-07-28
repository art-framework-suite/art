#include "art/Framework/Core/EDProducer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"


namespace art
{

  EDProducer::EDProducer()
    : ProducerBase()
    , EngineCreator()
    , moduleDescription_()
    , current_context_(0)
  { }

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

  void
  EDProducer::reconfigure(fhicl::ParameterSet const&) {
    mf::LogError("FeatureNotImplemented")
       << "This module is not reconfigurable."
       << "\n";
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
  EDProducer::doBeginSubRun(SubRunPrincipal & srp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun sr(srp, moduleDescription_);
    this->beginSubRun(sr);
    sr.commit_();
    return true;
  }

  bool
  EDProducer::doEndSubRun(SubRunPrincipal & srp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun sr(srp, moduleDescription_);
    this->endSubRun(sr);
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
    return current_context_;
  }

}  // art
