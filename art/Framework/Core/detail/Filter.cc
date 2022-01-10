#include "art/Framework/Core/detail/Filter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

using namespace std;

namespace art::detail {

  constexpr bool Filter::Pass;
  constexpr bool Filter::Fail;

  Filter::~Filter() noexcept = default;

  Filter::Filter(fhicl::ParameterSet const& pset)
    : checkPutProducts_{pset.get<bool>("errorOnFailureToPut", true)}
  {}

  void
  Filter::doBeginJob(SharedResources const& resources)
  {
    setupQueues(resources);
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
    auto r = Run::make(rp, mc, RangeSet::forRun(rp.runID()));
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = beginRunWithFrame(r, frame);
    r.commitProducts();
    return rc;
  }

  bool
  Filter::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    auto r = Run::make(rp, mc, rp.seenRanges());
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = endRunWithFrame(r, frame);
    r.commitProducts();
    return rc;
  }

  bool
  Filter::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    auto sr = SubRun::make(srp, mc, RangeSet::forSubRun(srp.subRunID()));
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = beginSubRunWithFrame(sr, frame);
    sr.commitProducts();
    return rc;
  }

  bool
  Filter::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    auto sr = SubRun::make(srp, mc, srp.seenRanges());
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = endSubRunWithFrame(sr, frame);
    sr.commitProducts();
    return rc;
  }

  bool
  Filter::doEvent(EventPrincipal& ep,
                  ModuleContext const& mc,
                  atomic<size_t>& counts_run,
                  atomic<size_t>& counts_passed,
                  atomic<size_t>& counts_failed)
  {
    auto e = Event::make(ep, mc);
    ++counts_run;
    ProcessingFrame const frame{mc.scheduleID()};
    bool const rc = filterWithFrame(e, frame);
    e.commitProducts(checkPutProducts_, &expectedProducts<InEvent>());
    if (rc) {
      ++counts_passed;
    } else {
      ++counts_failed;
    }
    return rc;
  }

} // namespace art::detail
