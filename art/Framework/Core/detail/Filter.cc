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
#include "art/Persistency/Provenance/ModuleContext.h"
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

    Filter::Filter(fhicl::ParameterSet const&)
    {
      // This constructor will eventually be used to query the
      // errorOnFailureToPut flag.
    }

    void
    Filter::doBeginJob()
    {
      setupQueues();
      failureToPutProducts(moduleDescription());
      Services const services{ScheduleID{}};
      beginJobWithServices(services);
    }

    void
    Filter::doEndJob()
    {
      Services const services{ScheduleID{}};
      endJobWithServices(services);
    }

    void
    Filter::doRespondToOpenInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenInputFileWithServices(fb, services);
    }

    void
    Filter::doRespondToCloseInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseInputFileWithServices(fb, services);
    }

    void
    Filter::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenOutputFilesWithServices(fb, services);
    }

    void
    Filter::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseOutputFilesWithServices(fb, services);
    }

    bool
    Filter::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run r{rp, mc, RangeSet::forRun(rp.runID())};
      Services const services{mc.scheduleID()};
      bool const rc = beginRunWithServices(r, services);
      r.movePutProductsToPrincipal(rp);
      return rc;
    }

    bool
    Filter::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run r{rp, mc, rp.seenRanges()};
      Services const services{mc.scheduleID()};
      bool const rc = endRunWithServices(r, services);
      r.movePutProductsToPrincipal(rp);
      return rc;
    }

    bool
    Filter::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun sr{srp, mc, RangeSet::forSubRun(srp.subRunID())};
      Services const services{mc.scheduleID()};
      bool const rc = beginSubRunWithServices(sr, services);
      sr.movePutProductsToPrincipal(srp);
      return rc;
    }

    bool
    Filter::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun sr{srp, mc, srp.seenRanges()};
      Services const services{mc.scheduleID()};
      bool const rc = endSubRunWithServices(sr, services);
      sr.movePutProductsToPrincipal(srp);
      return rc;
    }

    bool
    Filter::doEvent(EventPrincipal& ep,
                    ModuleContext const& mc,
                    atomic<size_t>& counts_run,
                    atomic<size_t>& counts_passed,
                    atomic<size_t>& counts_failed)
    {
      Event e{ep, mc};
      ++counts_run;
      Services const services{mc.scheduleID()};
      bool const rc = filterWithServices(e, services);
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
