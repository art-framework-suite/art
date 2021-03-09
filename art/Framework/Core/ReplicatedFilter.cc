#include "art/Framework/Core/ReplicatedFilter.h"
// vim: set sw=2 expandtab :

namespace art {

  std::string
  ReplicatedFilter::workerType() const
  {
    return "WorkerT<ReplicatedFilter>";
  }

  void
  ReplicatedFilter::setupQueues(detail::SharedResources const&)
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedFilter::respondToOpenInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  ReplicatedFilter::respondToCloseInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  ReplicatedFilter::respondToOpenOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  ReplicatedFilter::respondToCloseOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  ReplicatedFilter::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  ReplicatedFilter::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  bool
  ReplicatedFilter::beginRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    beginRun(r, frame);
    return true;
  }

  bool
  ReplicatedFilter::endRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    endRun(r, frame);
    return true;
  }

  bool
  ReplicatedFilter::beginSubRunWithFrame(SubRun& sr,
                                         ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
    return true;
  }

  bool
  ReplicatedFilter::endSubRunWithFrame(SubRun& sr, ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
    return true;
  }

  bool
  ReplicatedFilter::filterWithFrame(Event& e, ProcessingFrame const& frame)
  {
    return filter(e, frame);
  }

  // Default implementations
  void
  ReplicatedFilter::beginJob(ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::endJob(ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::respondToOpenInputFile(FileBlock const&,
                                           ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::respondToCloseInputFile(FileBlock const&,
                                            ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::respondToOpenOutputFiles(FileBlock const&,
                                             ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::respondToCloseOutputFiles(FileBlock const&,
                                              ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::beginRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::endRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::beginSubRun(SubRun const&, ProcessingFrame const&)
  {}

  void
  ReplicatedFilter::endSubRun(SubRun const&, ProcessingFrame const&)
  {}

} // namespace art
