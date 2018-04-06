#include "art/Framework/Core/EDProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {

  EDProducer::~EDProducer() noexcept = default;
  shared::Producer::~Producer() noexcept = default;
  replicated::Producer::~Producer() noexcept = default;

  EDProducer::EDProducer() = default;
  shared::Producer::Producer() = default;
  replicated::Producer::Producer() = default;

  string
  EDProducer::workerType() const
  {
    return "WorkerT<EDProducer>";
  }

  void
  EDProducer::reconfigure(fhicl::ParameterSet const&)
  {}

  void
  EDProducer::doRespondToOpenInputFile(FileBlock const& fb)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDProducer::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDProducer::doRespondToCloseInputFile(FileBlock const& fb)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDProducer::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDProducer::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDProducer::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDProducer::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDProducer::respondToCloseOutputFiles(FileBlock const&)
  {}

  void
  EDProducer::doBeginJob()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
    auto queues = SharedResourcesRegistry::instance()->createQueues(names);
    chain_.reset(new SerialTaskQueueChain{queues});
    auto const& mainID = moduleDescription().mainParameterSetID();
    auto const& scheduler_pset =
      fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
        "services.scheduler");
    auto const& module_pset =
      fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
    checkPutProducts_ =
      detail::get_failureToPut_flag(scheduler_pset, module_pset);
    beginJob();
  }

  void
  shared::Producer::doBeginJob()
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
      chain_.reset(new SerialTaskQueueChain{queues});
    }
    auto const& mainID = moduleDescription().mainParameterSetID();
    auto const& scheduler_pset =
      fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
        "services.scheduler");
    auto const& module_pset =
      fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
    checkPutProducts_ =
      detail::get_failureToPut_flag(scheduler_pset, module_pset);
    beginJob();
  }

  void
  replicated::Producer::doBeginJob()
  {
    auto const& mainID = moduleDescription().mainParameterSetID();
    auto const& scheduler_pset =
      fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
        "services.scheduler");
    auto const& module_pset =
      fhicl::ParameterSetRegistry::get(moduleDescription().parameterSetID());
    checkPutProducts_ =
      detail::get_failureToPut_flag(scheduler_pset, module_pset);
    beginJob();
  }

  void
  EDProducer::beginJob()
  {}

  void
  EDProducer::doEndJob()
  {
    endJob();
  }

  void
  EDProducer::endJob()
  {}

  bool
  EDProducer::doBeginRun(RunPrincipal& rp,
                         cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run r{rp,
          moduleDescription(),
          expectedProducts<InRun>(),
          RangeSet::forRun(rp.runID())};
    beginRun(r);
    r.DataViewImpl::commit(rp);
    return true;
  }

  void
  EDProducer::beginRun(Run&)
  {}

  bool
  EDProducer::doEndRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run r{rp, moduleDescription(), expectedProducts<InRun>(), rp.seenRanges()};
    endRun(r);
    r.DataViewImpl::commit(rp);
    return true;
  }

  void
  EDProducer::endRun(Run&)
  {}

  bool
  EDProducer::doBeginSubRun(SubRunPrincipal& srp,
                            cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun sr{srp,
              moduleDescription(),
              expectedProducts<InSubRun>(),
              RangeSet::forSubRun(srp.subRunID())};
    beginSubRun(sr);
    sr.DataViewImpl::commit(srp);
    return true;
  }

  void
  EDProducer::beginSubRun(SubRun&)
  {}

  bool
  EDProducer::doEndSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun sr{
      srp, moduleDescription(), expectedProducts<InSubRun>(), srp.seenRanges()};
    endSubRun(sr);
    sr.DataViewImpl::commit(srp);
    return true;
  }

  void
  EDProducer::endSubRun(SubRun&)
  {}

  bool
  EDProducer::doEvent(EventPrincipal& ep,
                      ScheduleID /*si*/,
                      CurrentProcessingContext const* cpc,
                      atomic<size_t>& counts_run,
                      atomic<size_t>& counts_passed,
                      atomic<size_t>& /*counts_failed*/)
  {
    detail::CPCSentry sentry{*cpc};
    Event e{ep, moduleDescription(), expectedProducts<InEvent>()};
    ++counts_run;
    produce(e);
    e.DataViewImpl::commit(ep, checkPutProducts_);
    ++counts_passed;
    return true;
  }

} // namespace art
