#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {

  EDAnalyzer::~EDAnalyzer() noexcept {}
  shared::Analyzer::~Analyzer() noexcept {}
  replicated::Analyzer::~Analyzer() noexcept {}

  EDAnalyzer::EDAnalyzer(fhicl::ParameterSet const& pset)
    : EventObserverBase{pset}
  {}

  shared::Analyzer::Analyzer(fhicl::ParameterSet const& pset)
    : art::EDAnalyzer{pset}
  {}

  replicated::Analyzer::Analyzer(fhicl::ParameterSet const& pset)
    : art::EDAnalyzer{pset}
  {}

  string
  EDAnalyzer::workerType() const
  {
    return "WorkerT<EDAnalyzer>";
  }

  // Not called by framework
  void
  EDAnalyzer::reconfigure(fhicl::ParameterSet const&)
  {}

  void
  EDAnalyzer::doBeginJob()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
    auto queues = SharedResourcesRegistry::instance()->createQueues(names);
    chain_ = new SerialTaskQueueChain{queues};
    beginJob();
  }

  void
  shared::Analyzer::doBeginJob()
  {
    if (!resourceNames_.empty()) {
      if (asyncDeclared_) {
        throw art::Exception{
          art::errors::LogicError,
          "An error occurred while processing scheduling options for a module."}
          << "async<InEvent>() cannot be called in combination with any "
             "serialize<InEvent>(...) calls.\n";
      }
      vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
      auto queues = SharedResourcesRegistry::instance()->createQueues(names);
      chain_ = new SerialTaskQueueChain{queues};
    }
    beginJob();
  }

  void
  replicated::Analyzer::doBeginJob()
  {
    beginJob();
  }

  void
  EDAnalyzer::beginJob()
  {}

  void
  EDAnalyzer::doEndJob()
  {
    endJob();
  }

  void
  EDAnalyzer::endJob()
  {}

  void
  EDAnalyzer::doRespondToOpenInputFile(FileBlock const& fb)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDAnalyzer::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::doRespondToCloseInputFile(FileBlock const& fb)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDAnalyzer::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDAnalyzer::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDAnalyzer::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDAnalyzer::respondToCloseOutputFiles(FileBlock const&)
  {}

  bool
  EDAnalyzer::doBeginRun(RunPrincipal& rp,
                         cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run const r{rp, md_};
    beginRun(r);
    return true;
  }

  void
  EDAnalyzer::beginRun(Run const&)
  {}

  bool
  EDAnalyzer::doEndRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run const r{rp, md_};
    endRun(r);
    return true;
  }

  void
  EDAnalyzer::endRun(Run const&)
  {}

  bool
  EDAnalyzer::doBeginSubRun(SubRunPrincipal& srp,
                            cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun const sr{srp, md_};
    beginSubRun(sr);
    return true;
  }

  void
  EDAnalyzer::beginSubRun(SubRun const&)
  {}

  bool
  EDAnalyzer::doEndSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun const sr{srp, md_};
    endSubRun(sr);
    return true;
  }

  void
  EDAnalyzer::endSubRun(SubRun const&)
  {}

  bool
  EDAnalyzer::doEvent(EventPrincipal& ep,
                      ScheduleID const /*si*/,
                      CurrentProcessingContext const* cpc,
                      std::atomic<std::size_t>& counts_run,
                      std::atomic<std::size_t>& counts_passed,
                      std::atomic<std::size_t>& /*counts_failed*/)
  {
    detail::CPCSentry sentry{*cpc};
    detail::PVSentry pvSentry{processAndEventSelectors()};
    Event const e{ep, md_};
    if (wantAllEvents() || wantEvent(e)) {
      ++counts_run;
      analyze(e);
      ++counts_passed;
    }
    return true;
  }

} // namespace art
