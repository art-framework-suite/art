#include "art/Framework/Core/detail/Filter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    constexpr bool Filter::Pass;
    constexpr bool Filter::Fail;

    Filter::Filter() = default;
    Filter::~Filter() noexcept = default;

    void
    Filter::doBeginJob()
    {
      setupQueues();
      failureToPutProducts(moduleDescription());
      beginJob();
    }

    void
    Filter::beginJob()
    {}

    void
    Filter::doEndJob()
    {
      endJob();
    }

    void
    Filter::endJob()
    {}

    void
    Filter::doRespondToOpenInputFile(FileBlock const& fb)
    {
      respondToOpenInputFile(fb);
    }

    void
    Filter::respondToOpenInputFile(FileBlock const&)
    {}

    void
    Filter::doRespondToCloseInputFile(FileBlock const& fb)
    {
      respondToCloseInputFile(fb);
    }

    void
    Filter::respondToCloseInputFile(FileBlock const&)
    {}

    void
    Filter::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      respondToOpenOutputFiles(fb);
    }

    void
    Filter::respondToOpenOutputFiles(FileBlock const&)
    {}

    void
    Filter::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      respondToCloseOutputFiles(fb);
    }

    void
    Filter::respondToCloseOutputFiles(FileBlock const&)
    {}

    bool
    Filter::doBeginRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, moduleDescription(), RangeSet::forRun(rp.runID())};
      bool const rc = beginRun(r);
      r.movePutProductsToPrincipal(rp);
      return rc;
    }

    bool
    Filter::beginRun(Run&)
    {
      return true;
    }

    bool
    Filter::doEndRun(RunPrincipal& rp,
                     cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, moduleDescription(), rp.seenRanges()};
      bool const rc = endRun(r);
      r.movePutProductsToPrincipal(rp);
      return rc;
    }

    bool
    Filter::endRun(Run&)
    {
      return true;
    }

    bool
    Filter::doBeginSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, moduleDescription(), RangeSet::forSubRun(srp.subRunID())};
      bool const rc = beginSubRun(sr);
      sr.movePutProductsToPrincipal(srp);
      return rc;
    }

    bool
    Filter::beginSubRun(SubRun&)
    {
      return true;
    }

    bool
    Filter::doEndSubRun(SubRunPrincipal& srp,
                        cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, moduleDescription(), srp.seenRanges()};
      bool const rc = endSubRun(sr);
      sr.movePutProductsToPrincipal(srp);
      return rc;
    }

    bool
    Filter::endSubRun(SubRun&)
    {
      return true;
    }

    bool
    Filter::doEvent(EventPrincipal& ep,
                    ScheduleID const sid,
                    ModuleContext const& mc [[gnu::unused]],
                    atomic<size_t>& counts_run,
                    atomic<size_t>& counts_passed,
                    atomic<size_t>& counts_failed)
    {
      //      detail::CPCSentry sentry{*cpc};
      Event e{ep, moduleDescription()};
      ++counts_run;
      bool rc = false;
      rc = filterWithScheduleID(e, sid);
      e.movePutProductsToPrincipal(
        ep, checkPutProducts_, &expectedProducts<InEvent>());
      if (rc) {
        ++counts_passed;
      } else {
        ++counts_failed;
      }
      return rc;
    }

    void
    Filter::failureToPutProducts(ModuleDescription const& md)
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
