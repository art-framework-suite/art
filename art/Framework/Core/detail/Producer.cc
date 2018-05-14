#include "art/Framework/Core/detail/Producer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    Producer::Producer() = default;
    Producer::~Producer() noexcept = default;

    void
    Producer::doRespondToOpenInputFile(FileBlock const& fb)
    {
      respondToOpenInputFile(fb);
    }

    void
    Producer::respondToOpenInputFile(FileBlock const&)
    {}

    void
    Producer::doRespondToCloseInputFile(FileBlock const& fb)
    {
      respondToCloseInputFile(fb);
    }

    void
    Producer::respondToCloseInputFile(FileBlock const&)
    {}

    void
    Producer::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      respondToOpenOutputFiles(fb);
    }

    void
    Producer::respondToOpenOutputFiles(FileBlock const&)
    {}

    void
    Producer::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      respondToCloseOutputFiles(fb);
    }

    void
    Producer::respondToCloseOutputFiles(FileBlock const&)
    {}

    void
    Producer::doBeginJob()
    {
      setupQueues();
      failureToPutProducts(moduleDescription());
      beginJob();
    }

    void
    Producer::beginJob()
    {}

    void
    Producer::doEndJob()
    {
      endJob();
    }

    void
    Producer::endJob()
    {}

    bool
    Producer::doBeginRun(RunPrincipal& rp,
                         ModuleContext const& mc[[gnu::unused]])
    {
      Run r{rp, moduleDescription(), RangeSet::forRun(rp.runID())};
      beginRun(r);
      r.movePutProductsToPrincipal(rp);
      return true;
    }

    void
    Producer::beginRun(Run&)
    {}

    bool
    Producer::doEndRun(RunPrincipal& rp, ModuleContext const& mc[[gnu::unused]])
    {
      Run r{rp, moduleDescription(), rp.seenRanges()};
      endRun(r);
      r.movePutProductsToPrincipal(rp);
      return true;
    }

    void
    Producer::endRun(Run&)
    {}

    bool
    Producer::doBeginSubRun(SubRunPrincipal& srp,
                            ModuleContext const& mc[[gnu::unused]])
    {
      SubRun sr{srp, moduleDescription(), RangeSet::forSubRun(srp.subRunID())};
      beginSubRun(sr);
      sr.movePutProductsToPrincipal(srp);
      return true;
    }

    void
    Producer::beginSubRun(SubRun&)
    {}

    bool
    Producer::doEndSubRun(SubRunPrincipal& srp,
                          ModuleContext const& mc[[gnu::unused]])
    {
      SubRun sr{srp, moduleDescription(), srp.seenRanges()};
      endSubRun(sr);
      sr.movePutProductsToPrincipal(srp);
      return true;
    }

    void
    Producer::endSubRun(SubRun&)
    {}

    bool
    Producer::doEvent(EventPrincipal& ep,
                      ModuleContext const& mc,
                      std::atomic<size_t>& counts_run,
                      std::atomic<size_t>& counts_passed,
                      std::atomic<size_t>& /*counts_failed*/)
    {
      Event e{ep, moduleDescription()};
      ++counts_run;
      produceWithScheduleID(e, mc.scheduleID());
      e.movePutProductsToPrincipal(
        ep, checkPutProducts_, &expectedProducts<InEvent>());
      ++counts_passed;
      return true;
    }

    void
    Producer::failureToPutProducts(ModuleDescription const& md)
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
