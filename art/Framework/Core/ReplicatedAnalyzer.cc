#include "art/Framework/Core/ReplicatedAnalyzer.h"
// vim: set sw=2 expandtab :

namespace art {

  void
  ReplicatedAnalyzer::setupQueues(detail::SharedResources const&)
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedAnalyzer::respondToOpenInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  ReplicatedAnalyzer::respondToCloseInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  ReplicatedAnalyzer::respondToOpenOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  ReplicatedAnalyzer::respondToCloseOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  ReplicatedAnalyzer::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  ReplicatedAnalyzer::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  void
  ReplicatedAnalyzer::beginRunWithFrame(Run const& r,
                                        ProcessingFrame const& frame)
  {
    beginRun(r, frame);
  }

  void
  ReplicatedAnalyzer::endRunWithFrame(Run const& r,
                                      ProcessingFrame const& frame)
  {
    endRun(r, frame);
  }

  void
  ReplicatedAnalyzer::beginSubRunWithFrame(SubRun const& sr,
                                           ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
  }

  void
  ReplicatedAnalyzer::endSubRunWithFrame(SubRun const& sr,
                                         ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
  }

  void
  ReplicatedAnalyzer::analyzeWithFrame(Event const& e,
                                       ProcessingFrame const& frame)
  {
    analyze(e, frame);
  }

  void
  ReplicatedAnalyzer::beginJob(ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::endJob(ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::respondToOpenInputFile(FileBlock const&,
                                             ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::respondToCloseInputFile(FileBlock const&,
                                              ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::respondToOpenOutputFiles(FileBlock const&,
                                               ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::respondToCloseOutputFiles(FileBlock const&,
                                                ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::beginRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::endRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::beginSubRun(SubRun const&, ProcessingFrame const&)
  {}

  void
  ReplicatedAnalyzer::endSubRun(SubRun const&, ProcessingFrame const&)
  {}

} // namespace art
