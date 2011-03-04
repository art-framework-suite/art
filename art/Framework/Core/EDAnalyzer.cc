#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/SubRun.h"


namespace art
{

  EDAnalyzer::~EDAnalyzer()
  { }

  bool
  EDAnalyzer::doEvent(EventPrincipal const& ep,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Event e(const_cast<EventPrincipal &>(ep), moduleDescription_);
    this->analyze(e);
    return true;
  }

  void
  EDAnalyzer::doBeginJob() {
    this->beginJob();
  }

  void
  EDAnalyzer::doEndJob() {
    this->endJob();
  }

  void 
  EDAnalyzer::reconfigure(ParameterSet const&) {
     mf::LogError("FeatureNotImplemented")
        << "This module is not reconfigurable."
        << "\n";
   }

  bool
  EDAnalyzer::doBeginRun(RunPrincipal const& rp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Run r(const_cast<RunPrincipal &>(rp), moduleDescription_);
    this->beginRun(r);
    return true;
  }

  bool
  EDAnalyzer::doEndRun(RunPrincipal const& rp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    Run r(const_cast<RunPrincipal &>(rp), moduleDescription_);
    this->endRun(r);
    return true;
  }

  bool
  EDAnalyzer::doBeginSubRun(SubRunPrincipal const& srp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun sr(const_cast<SubRunPrincipal &>(srp), moduleDescription_);
    this->beginSubRun(sr);
    return true;
  }

  bool
  EDAnalyzer::doEndSubRun(SubRunPrincipal const& srp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun sr(const_cast<SubRunPrincipal &>(srp), moduleDescription_);
    this->endSubRun(sr);
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
    return current_context_;
  }

}  // art
