#include "art/Framework/Core/detail/Producer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/ScheduleID.h"

namespace art::detail {

  Producer::~Producer() noexcept = default;

  Producer::Producer(fhicl::ParameterSet const& pset)
    : checkPutProducts_{pset.get<bool>("errorOnFailureToPut", true)}
  {}

  void
  Producer::doBeginJob(SharedResources const& resources)
  {
    setupQueues(resources);
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
    auto r = Run::make(rp, mc, RangeSet::forRun(rp.runID()));
    ProcessingFrame const frame{mc.scheduleID()};
    beginRunWithFrame(r, frame);
    r.commitProducts();
    return true;
  }

  bool
  Producer::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    auto r = Run::make(rp, mc, rp.seenRanges());
    ProcessingFrame const frame{mc.scheduleID()};
    endRunWithFrame(r, frame);
    r.commitProducts();
    return true;
  }

  bool
  Producer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    auto sr = SubRun::make(srp, mc, RangeSet::forSubRun(srp.subRunID()));
    ProcessingFrame const frame{mc.scheduleID()};
    beginSubRunWithFrame(sr, frame);
    sr.commitProducts();
    return true;
  }

  bool
  Producer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    auto sr = SubRun::make(srp, mc, srp.seenRanges());
    ProcessingFrame const frame{mc.scheduleID()};
    endSubRunWithFrame(sr, frame);
    sr.commitProducts();
    return true;
  }

  bool
  Producer::doEvent(EventPrincipal& ep,
                    ModuleContext const& mc,
                    std::atomic<size_t>& counts_run,
                    std::atomic<size_t>& counts_passed,
                    std::atomic<size_t>& /*counts_failed*/)
  {
    auto e = Event::make(ep, mc);
    ++counts_run;
    ProcessingFrame const frame{mc.scheduleID()};
    produceWithFrame(e, frame);
    e.commitProducts(checkPutProducts_, &expectedProducts<InEvent>());
    ++counts_passed;
    return true;
  }

} // namespace art::detail
