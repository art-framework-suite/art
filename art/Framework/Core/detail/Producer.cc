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

namespace art::detail {

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
    ProcessingFrame const frame{ScheduleID{}};
    beginJobWithFrame(frame);
  }

  void
  Producer::doEndJob()
  {
    ProcessingFrame const frame{ScheduleID{}};
    endJobWithFrame(frame);
  }

  void
  Producer::doRespondToOpenInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenInputFileWithFrame(fb, frame);
  }

  void
  Producer::doRespondToCloseInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseInputFileWithFrame(fb, frame);
  }

  void
  Producer::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenOutputFilesWithFrame(fb, frame);
  }

  void
  Producer::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseOutputFilesWithFrame(fb, frame);
  }

  bool
  Producer::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run r{rp, mc, RangeSet::forRun(rp.runID())};
    ProcessingFrame const frame{mc.scheduleID()};
    beginRunWithFrame(r, frame);
    r.movePutProductsToPrincipal(rp);
    return true;
  }

  bool
  Producer::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run r{rp, mc, rp.seenRanges()};
    ProcessingFrame const frame{mc.scheduleID()};
    endRunWithFrame(r, frame);
    r.movePutProductsToPrincipal(rp);
    return true;
  }

  bool
  Producer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun sr{srp, mc, RangeSet::forSubRun(srp.subRunID())};
    ProcessingFrame const frame{mc.scheduleID()};
    beginSubRunWithFrame(sr, frame);
    sr.movePutProductsToPrincipal(srp);
    return true;
  }

  bool
  Producer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun sr{srp, mc, srp.seenRanges()};
    ProcessingFrame const frame{mc.scheduleID()};
    endSubRunWithFrame(sr, frame);
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
    ProcessingFrame const frame{mc.scheduleID()};
    produceWithFrame(e, frame);
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

} // namespace art::detail
