#include "art/Framework/Core/SharedAnalyzer.h"
// vim: set sw=2 expandtab :

namespace art {

  SharedAnalyzer::SharedAnalyzer(fhicl::ParameterSet const& pset)
    : Analyzer{pset}, SharedModule{pset.get<std::string>("module_label")}
  {}

  std::unique_ptr<Worker>
  SharedAnalyzer::doMakeWorker(WorkerParams const& wp)
  {
    return std::make_unique<WorkerT<SharedAnalyzer>>(this, wp);
  }

  void
  SharedAnalyzer::setupQueues(detail::SharedResources const& resources)
  {
    createQueues(resources);
  }

  void
  SharedAnalyzer::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                                  ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  SharedAnalyzer::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                                   ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  SharedAnalyzer::respondToOpenOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  SharedAnalyzer::respondToCloseOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  SharedAnalyzer::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  SharedAnalyzer::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  void
  SharedAnalyzer::beginRunWithFrame(Run const& r, ProcessingFrame const& frame)
  {
    beginRun(r, frame);
  }

  void
  SharedAnalyzer::endRunWithFrame(Run const& r, ProcessingFrame const& frame)
  {
    endRun(r, frame);
  }

  void
  SharedAnalyzer::beginSubRunWithFrame(SubRun const& sr,
                                       ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
  }

  void
  SharedAnalyzer::endSubRunWithFrame(SubRun const& sr,
                                     ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
  }

  void
  SharedAnalyzer::analyzeWithFrame(Event const& e, ProcessingFrame const& frame)
  {
    analyze(e, frame);
  }

  void
  SharedAnalyzer::beginJob(ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::endJob(ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::respondToOpenInputFile(FileBlock const&,
                                         ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::respondToCloseInputFile(FileBlock const&,
                                          ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::respondToOpenOutputFiles(FileBlock const&,
                                           ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::respondToCloseOutputFiles(FileBlock const&,
                                            ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::beginRun(Run const&, ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::endRun(Run const&, ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::beginSubRun(SubRun const&, ProcessingFrame const&)
  {}

  void
  SharedAnalyzer::endSubRun(SubRun const&, ProcessingFrame const&)
  {}

} // namespace art
