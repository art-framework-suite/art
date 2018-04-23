#include "art/Framework/Core/detail/ProducerImpl.h"
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
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    ProducerImpl::ProducerImpl() = default;
    ProducerImpl::~ProducerImpl() noexcept = default;

    void
    ProducerImpl::doRespondToOpenInputFile(FileBlock const& fb)
    {
      respondToOpenInputFile(fb);
    }

    void
    ProducerImpl::respondToOpenInputFile(FileBlock const&)
    {}

    void
    ProducerImpl::doRespondToCloseInputFile(FileBlock const& fb)
    {
      respondToCloseInputFile(fb);
    }

    void
    ProducerImpl::respondToCloseInputFile(FileBlock const&)
    {}

    void
    ProducerImpl::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      respondToOpenOutputFiles(fb);
    }

    void
    ProducerImpl::respondToOpenOutputFiles(FileBlock const&)
    {}

    void
    ProducerImpl::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      respondToCloseOutputFiles(fb);
    }

    void
    ProducerImpl::respondToCloseOutputFiles(FileBlock const&)
    {}

    void
    ProducerImpl::beginJob()
    {}

    void
    ProducerImpl::doEndJob()
    {
      endJob();
    }

    void
    ProducerImpl::endJob()
    {}

    bool
    ProducerImpl::doBeginRun(RunPrincipal& rp,
                             cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, md_, RangeSet::forRun(rp.runID())};
      beginRun(r);
      r.DataViewImpl::movePutProductsToPrincipal(rp);
      return true;
    }

    void
    ProducerImpl::beginRun(Run&)
    {}

    bool
    ProducerImpl::doEndRun(RunPrincipal& rp,
                           cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, md_, rp.seenRanges()};
      endRun(r);
      r.DataViewImpl::movePutProductsToPrincipal(rp);
      return true;
    }

    void
    ProducerImpl::endRun(Run&)
    {}

    bool
    ProducerImpl::doBeginSubRun(SubRunPrincipal& srp,
                                cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, md_, RangeSet::forSubRun(srp.subRunID())};
      beginSubRun(sr);
      sr.DataViewImpl::movePutProductsToPrincipal(srp);
      return true;
    }

    void
    ProducerImpl::beginSubRun(SubRun&)
    {}

    bool
    ProducerImpl::doEndSubRun(SubRunPrincipal& srp,
                              cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, md_, srp.seenRanges()};
      endSubRun(sr);
      sr.DataViewImpl::movePutProductsToPrincipal(srp);
      return true;
    }

    void
    ProducerImpl::endSubRun(SubRun&)
    {}

    bool
    ProducerImpl::doEvent(EventPrincipal& ep,
                          ScheduleID const /*si*/,
                          CurrentProcessingContext const* cpc,
                          std::atomic<size_t>& counts_run,
                          std::atomic<size_t>& counts_passed,
                          std::atomic<size_t>& /*counts_failed*/)
    {
      detail::CPCSentry sentry{*cpc};
      Event e{ep, md_};
      ++counts_run;
      produce(e);
      e.DataViewImpl::movePutProductsToPrincipal(
                                                 ep, checkPutProducts_, &expectedProducts<InEvent>());
      ++counts_passed;
      return true;
    }

    void
    ProducerImpl::failureToPutProducts(ModuleDescription const& md)
    {
      auto const& mainID = md.mainParameterSetID();
      auto const& scheduler_pset =
        fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
                                                                          "services.scheduler");
      auto const& module_pset =
        fhicl::ParameterSetRegistry::get(md.parameterSetID());
      checkPutProducts_ =
        detail::get_failureToPut_flag(scheduler_pset, module_pset);
    }

  } // namespace detail
} // namespace art
