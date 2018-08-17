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

namespace art::detail {

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
    ProcessingFrame const frame{ScheduleID{}};
    beginJobWithFrame(frame);
  }

  void
  Filter::doEndJob()
  {
    ProcessingFrame const frame{ScheduleID{}};
    endJobWithFrame(frame);
  }

  void
  Filter::doRespondToOpenInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenInputFileWithFrame(fb, frame);
  }

  void
  Filter::doRespondToCloseInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseInputFileWithFrame(fb, frame);
  }

  void
  Filter::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenOutputFilesWithFrame(fb, frame);
  }

  void
  Filter::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseOutputFilesWithFrame(fb, frame);
  }

  bool
  Filter::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run r{rp, mc, RangeSet::forRun(rp.runID())};
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = beginRunWithFrame(r, frame);
    r.movePutProductsToPrincipal(rp);
    return rc;
  }

  bool
  Filter::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run r{rp, mc, rp.seenRanges()};
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = endRunWithFrame(r, frame);
    r.movePutProductsToPrincipal(rp);
    return rc;
  }

  bool
  Filter::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun sr{srp, mc, RangeSet::forSubRun(srp.subRunID())};
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = beginSubRunWithFrame(sr, frame);
    sr.movePutProductsToPrincipal(srp);
    return rc;
  }

  bool
  Filter::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun sr{srp, mc, srp.seenRanges()};
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = endSubRunWithFrame(sr, frame);
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
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = filterWithFrame(e, frame);
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

} // namespace art::detail
