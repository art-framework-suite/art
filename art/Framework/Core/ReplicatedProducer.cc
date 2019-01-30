#include "art/Framework/Core/ReplicatedProducer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  ReplicatedProducer::workerType() const
  {
    return "WorkerT<ReplicatedProducer>";
  }

  void
  ReplicatedProducer::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedProducer::respondToOpenInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  ReplicatedProducer::respondToCloseInputFileWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  ReplicatedProducer::respondToOpenOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  ReplicatedProducer::respondToCloseOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  ReplicatedProducer::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  ReplicatedProducer::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  void
  ReplicatedProducer::beginRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    beginRun(r, frame);
  }

  void
  ReplicatedProducer::endRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    endRun(r, frame);
  }

  void
  ReplicatedProducer::beginSubRunWithFrame(SubRun& sr,
                                           ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
  }

  void
  ReplicatedProducer::endSubRunWithFrame(SubRun& sr,
                                         ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
  }

  void
  ReplicatedProducer::produceWithFrame(Event& e, ProcessingFrame const& frame)
  {
    produce(e, frame);
  }

  // Default implementations
  void
  ReplicatedProducer::beginJob(ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::endJob(ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::respondToOpenInputFile(FileBlock const&,
                                             ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::respondToCloseInputFile(FileBlock const&,
                                              ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::respondToOpenOutputFiles(FileBlock const&,
                                               ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::respondToCloseOutputFiles(FileBlock const&,
                                                ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::beginRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::endRun(Run const&, ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::beginSubRun(SubRun const&, ProcessingFrame const&)
  {}

  void
  ReplicatedProducer::endSubRun(SubRun const&, ProcessingFrame const&)
  {}

} // namespace art
