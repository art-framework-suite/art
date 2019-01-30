#include "art/Framework/Core/EDProducer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  EDProducer::workerType() const
  {
    return "WorkerT<EDProducer>";
  }

  void
  EDProducer::setupQueues()
  {
    createQueues();
  }

  void
  EDProducer::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                              ProcessingFrame const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDProducer::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                               ProcessingFrame const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDProducer::respondToOpenOutputFilesWithFrame(FileBlock const& fb,
                                                ProcessingFrame const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDProducer::respondToCloseOutputFilesWithFrame(FileBlock const& fb,
                                                 ProcessingFrame const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDProducer::beginJobWithFrame(ProcessingFrame const&)
  {
    beginJob();
  }

  void
  EDProducer::endJobWithFrame(ProcessingFrame const&)
  {
    endJob();
  }

  void
  EDProducer::beginRunWithFrame(Run& r, ProcessingFrame const&)
  {
    beginRun(r);
  }

  void
  EDProducer::endRunWithFrame(Run& r, ProcessingFrame const&)
  {
    endRun(r);
  }

  void
  EDProducer::beginSubRunWithFrame(SubRun& sr, ProcessingFrame const&)
  {
    beginSubRun(sr);
  }

  void
  EDProducer::endSubRunWithFrame(SubRun& sr, ProcessingFrame const&)
  {
    endSubRun(sr);
  }

  void
  EDProducer::produceWithFrame(Event& e, ProcessingFrame const& frame)
  {
    ScheduleIDSentry sentry{*this, frame.scheduleID()};
    produce(e);
  }

  // Default implementations
  void
  EDProducer::beginJob()
  {}

  void
  EDProducer::endJob()
  {}

  void
  EDProducer::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDProducer::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDProducer::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDProducer::respondToCloseOutputFiles(FileBlock const&)
  {}

  void
  EDProducer::beginRun(Run&)
  {}

  void
  EDProducer::endRun(Run&)
  {}

  void
  EDProducer::beginSubRun(SubRun&)
  {}

  void
  EDProducer::endSubRun(SubRun&)
  {}

} // namespace art
