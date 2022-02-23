#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

namespace art {

  void
  EDAnalyzer::setupQueues(detail::SharedResources const& resources)
  {
    createQueues(resources);
  }

  void
  EDAnalyzer::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                              ProcessingFrame const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDAnalyzer::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                               ProcessingFrame const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDAnalyzer::respondToOpenOutputFilesWithFrame(FileBlock const& fb,
                                                ProcessingFrame const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDAnalyzer::respondToCloseOutputFilesWithFrame(FileBlock const& fb,
                                                 ProcessingFrame const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDAnalyzer::beginJobWithFrame(ProcessingFrame const&)
  {
    beginJob();
  }

  void
  EDAnalyzer::endJobWithFrame(ProcessingFrame const&)
  {
    endJob();
  }

  void
  EDAnalyzer::beginRunWithFrame(Run const& r, ProcessingFrame const&)
  {
    beginRun(r);
  }

  void
  EDAnalyzer::endRunWithFrame(Run const& r, ProcessingFrame const&)
  {
    endRun(r);
  }

  void
  EDAnalyzer::beginSubRunWithFrame(SubRun const& sr, ProcessingFrame const&)
  {
    beginSubRun(sr);
  }

  void
  EDAnalyzer::endSubRunWithFrame(SubRun const& sr, ProcessingFrame const&)
  {
    endSubRun(sr);
  }

  void
  EDAnalyzer::analyzeWithFrame(Event const& e, ProcessingFrame const& frame)
  {
    ScheduleIDSentry sentry{*this, frame.scheduleID()};
    analyze(e);
  }

  // Default implementations
  void
  EDAnalyzer::beginJob()
  {}

  void
  EDAnalyzer::endJob()
  {}

  void
  EDAnalyzer::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToCloseOutputFiles(FileBlock const&)
  {}

  void
  EDAnalyzer::beginRun(Run const&)
  {}

  void
  EDAnalyzer::endRun(Run const&)
  {}

  void
  EDAnalyzer::beginSubRun(SubRun const&)
  {}

  void
  EDAnalyzer::endSubRun(SubRun const&)
  {}

  template class WorkerT<EDAnalyzer>;

} // namespace art
