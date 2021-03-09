#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleContext.h"

#include <ostream>

using namespace hep::concurrency;
using namespace std;

using art::SharedResources;

namespace art::detail {

  Analyzer::~Analyzer() noexcept = default;

  Analyzer::Analyzer(fhicl::ParameterSet const& pset) : Observer{pset} {}

  void
  Analyzer::doBeginJob(SharedResources const& resources)
  {
    setupQueues(resources);
    ProcessingFrame const frame{ScheduleID{}};
    beginJobWithFrame(frame);
  }

  void
  Analyzer::doEndJob()
  {
    ProcessingFrame const frame{ScheduleID{}};
    endJobWithFrame(frame);
  }

  void
  Analyzer::doRespondToOpenInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenInputFileWithFrame(fb, frame);
  }

  void
  Analyzer::doRespondToCloseInputFile(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseInputFileWithFrame(fb, frame);
  }

  void
  Analyzer::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToOpenOutputFilesWithFrame(fb, frame);
  }

  void
  Analyzer::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    ProcessingFrame const frame{ScheduleID{}};
    respondToCloseOutputFilesWithFrame(fb, frame);
  }

  bool
  Analyzer::doBeginRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run const r{rp, mc};
    ProcessingFrame const frame{mc.scheduleID()};
    beginRunWithFrame(r, frame);
    return true;
  }

  bool
  Analyzer::doEndRun(RunPrincipal& rp, ModuleContext const& mc)
  {
    Run const r{rp, mc};
    ProcessingFrame const frame{mc.scheduleID()};
    endRunWithFrame(r, frame);
    return true;
  }

  bool
  Analyzer::doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun const sr{srp, mc};
    ProcessingFrame const frame{mc.scheduleID()};
    beginSubRunWithFrame(sr, frame);
    return true;
  }

  bool
  Analyzer::doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    SubRun const sr{srp, mc};
    ProcessingFrame const frame{mc.scheduleID()};
    endSubRunWithFrame(sr, frame);
    return true;
  }

  bool
  Analyzer::doEvent(EventPrincipal& ep,
                    ModuleContext const& mc,
                    std::atomic<std::size_t>& counts_run,
                    std::atomic<std::size_t>& counts_passed,
                    std::atomic<std::size_t>& /*counts_failed*/)
  {
    Event const e{ep, mc};
    if (wantEvent(e)) {
      ++counts_run;
      ProcessingFrame const frame{mc.scheduleID()};
      analyzeWithFrame(e, frame);
      ++counts_passed;
    }
    return true;
  }

} // namespace art::detail
