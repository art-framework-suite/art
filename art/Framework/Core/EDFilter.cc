#include "art/Framework/Core/EDFilter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {

  constexpr bool EDFilter::Pass;
  constexpr bool EDFilter::Fail;

  EDFilter::EDFilter() = default;
  shared::Filter::Filter() = default;
  replicated::Filter::Filter() = default;

  EDFilter::~EDFilter() noexcept = default;
  shared::Filter::~Filter() noexcept = default;
  replicated::Filter::~Filter() noexcept = default;

  string
  EDFilter::workerType() const
  {
    return "WorkerT<EDFilter>";
  }

  void
  EDFilter::reconfigure(fhicl::ParameterSet const&)
  {}

  void
  EDFilter::doBeginJob()
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
  shared::Filter::doBeginJob()
  {
    if (!resourceNames_.empty()) {
      if (asyncDeclared_) {
        throw art::Exception{art::errors::LogicError,
            "An error occurred while processing scheduling options for a module."}
        << "async<InEvent>() cannot be called in combination with any serialize<InEvent>(...) calls.\n";
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
  replicated::Filter::doBeginJob()
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
  EDFilter::beginJob()
  {}

  void
  EDFilter::doEndJob()
  {
    endJob();
  }

  void
  EDFilter::endJob()
  {}

  void
  EDFilter::doRespondToOpenInputFile(FileBlock const& fb)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDFilter::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDFilter::doRespondToCloseInputFile(FileBlock const& fb)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDFilter::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDFilter::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDFilter::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDFilter::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDFilter::respondToCloseOutputFiles(FileBlock const&)
  {}

  bool
  EDFilter::doBeginRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run r{rp,
          moduleDescription(),
          expectedProducts<InRun>(),
          RangeSet::forRun(rp.runID())};
    bool const rc = beginRun(r);
    r.DataViewImpl::commit(rp);
    return rc;
  }

  bool
  EDFilter::beginRun(Run&)
  {
    return true;
  }

  bool
  EDFilter::doEndRun(RunPrincipal& rp,
                     cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    Run r{rp, moduleDescription(), expectedProducts<InRun>(), rp.seenRanges()};
    bool const rc = endRun(r);
    r.DataViewImpl::commit(rp);
    return rc;
  }

  bool
  EDFilter::endRun(Run&)
  {
    return true;
  }

  bool
  EDFilter::doBeginSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun sr{srp,
              moduleDescription(),
              expectedProducts<InSubRun>(),
              RangeSet::forSubRun(srp.subRunID())};
    bool const rc = beginSubRun(sr);
    sr.DataViewImpl::commit(srp);
    return rc;
  }

  bool
  EDFilter::beginSubRun(SubRun&)
  {
    return true;
  }

  bool
  EDFilter::doEndSubRun(SubRunPrincipal& srp,
                        cet::exempt_ptr<CurrentProcessingContext const> cpc)
  {
    detail::CPCSentry sentry{*cpc};
    SubRun sr{
      srp, moduleDescription(), expectedProducts<InSubRun>(), srp.seenRanges()};
    bool const rc = endSubRun(sr);
    sr.DataViewImpl::commit(srp);
    return rc;
  }

  bool
  EDFilter::endSubRun(SubRun&)
  {
    return true;
  }

  bool
  EDFilter::doEvent(EventPrincipal& ep,
                    ScheduleID /*si*/,
                    CurrentProcessingContext const* cpc,
                    atomic<size_t>& counts_run,
                    atomic<size_t>& counts_passed,
                    atomic<size_t>& counts_failed)
  {
    detail::CPCSentry sentry{*cpc};
    Event e{ep, moduleDescription(), expectedProducts<InEvent>()};
    ++counts_run;
    bool rc = false;
    rc = filter(e);
    e.DataViewImpl::commit(ep, checkPutProducts_);
    if (rc) {
      ++counts_passed;
    } else {
      ++counts_failed;
    }
    return rc;
  }

} // namespace art
