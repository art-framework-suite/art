/*----------------------------------------------------------------------

----------------------------------------------------------------------*/


#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/SubRun.h"
#include "art/ParameterSet/ParameterSetDescription.h"


namespace edm
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
  EDAnalyzer::doBeginSubRun(SubRunPrincipal const& lbp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun lb(const_cast<SubRunPrincipal &>(lbp), moduleDescription_);
    this->beginSubRun(lb);
    return true;
  }

  bool
  EDAnalyzer::doEndSubRun(SubRunPrincipal const& lbp,
                        CurrentProcessingContext const* cpc) {
    detail::CPCSentry sentry(current_context_, cpc);
    SubRun lb(const_cast<SubRunPrincipal &>(lbp), moduleDescription_);
    this->endSubRun(lb);
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

  void
  EDAnalyzer::fillDescription(edm::ParameterSetDescription& iDesc,
                              std::string const& moduleLabel) {
    iDesc.setUnknown();
  }

}  // namespace edm
