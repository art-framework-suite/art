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

    Producer::Producer(fhicl::ParameterSet const&)
    {
      // This constructor will eventually be used to query the
      // errorOnFailureToPut flag.
    }

    void
    Producer::doBeginJob()
    {
      setupQueues();
      failureToPutProducts(moduleDescription());
      Services const services{ScheduleID{}};
      beginJobWithServices(services);
    }

    void
    Producer::doEndJob()
    {
      Services const services{ScheduleID{}};
      endJobWithServices(services);
    }

    void
    Producer::doRespondToOpenInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenInputFileWithServices(fb, services);
    }

    void
    Producer::doRespondToCloseInputFile(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseInputFileWithServices(fb, services);
    }

    void
    Producer::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToOpenOutputFilesWithServices(fb, services);
    }

    void
    Producer::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      Services const services{ScheduleID{}};
      respondToCloseOutputFilesWithServices(fb, services);
    }

    bool
    Producer::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run r{rp, mc, RangeSet::forRun(rp.runID())};
      Services const services{mc.scheduleID()};
      beginRunWithServices(r, services);
      r.movePutProductsToPrincipal(rp);
      return true;
    }

    bool
    Producer::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
    {
      Run r{rp, mc, rp.seenRanges()};
      Services const services{mc.scheduleID()};
      endRunWithServices(r, services);
      r.movePutProductsToPrincipal(rp);
      return true;
    }

    bool
    Producer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun sr{srp, mc, RangeSet::forSubRun(srp.subRunID())};
      Services const services{mc.scheduleID()};
      beginSubRunWithServices(sr, services);
      sr.movePutProductsToPrincipal(srp);
      return true;
    }

    bool
    Producer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
    {
      SubRun sr{srp, mc, srp.seenRanges()};
      Services const services{mc.scheduleID()};
      endSubRunWithServices(sr, services);
      sr.movePutProductsToPrincipal(srp);
      return true;
    }

    bool
    Producer::doEvent(EventPrincipal& ep,
                      ModuleContext const& mc,
                      std::atomic<size_t>& counts_run,
                      std::atomic<size_t>& counts_passed,
                      std::atomic<size_t>& /*counts_failed*/)
    {
      Event e{ep, mc};
      ++counts_run;
      Services const services{mc.scheduleID()};
      produceWithServices(e, services);
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
