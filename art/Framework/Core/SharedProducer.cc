#include "art/Framework/Core/SharedProducer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  void
  SharedProducer::setupQueues(detail::SharedResources const& resources)
  {
    createQueues(resources);
  }

  void
  SharedProducer::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                                  ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  SharedProducer::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                                   ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  SharedProducer::respondToOpenOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  SharedProducer::respondToCloseOutputFilesWithFrame(
    FileBlock const& fb,
    ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  SharedProducer::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  SharedProducer::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  void
  SharedProducer::beginRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    beginRun(r, frame);
  }

  void
  SharedProducer::endRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    endRun(r, frame);
  }

  void
  SharedProducer::beginSubRunWithFrame(SubRun& sr, ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
  }

  void
  SharedProducer::endSubRunWithFrame(SubRun& sr, ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
  }

  void
  SharedProducer::produceWithFrame(Event& e, ProcessingFrame const& frame)
  {
    produce(e, frame);
  }

  // Default implementations
  void
  SharedProducer::beginJob(ProcessingFrame const&)
  {}

  void
  SharedProducer::endJob(ProcessingFrame const&)
  {}

  void
  SharedProducer::respondToOpenInputFile(FileBlock const&,
                                         ProcessingFrame const&)
  {}

  void
  SharedProducer::respondToCloseInputFile(FileBlock const&,
                                          ProcessingFrame const&)
  {}

  void
  SharedProducer::respondToOpenOutputFiles(FileBlock const&,
                                           ProcessingFrame const&)
  {}

  void
  SharedProducer::respondToCloseOutputFiles(FileBlock const&,
                                            ProcessingFrame const&)
  {}

  void
  SharedProducer::beginRun(Run&, ProcessingFrame const&)
  {}

  void
  SharedProducer::endRun(Run&, ProcessingFrame const&)
  {}

  void
  SharedProducer::beginSubRun(SubRun&, ProcessingFrame const&)
  {}

  void
  SharedProducer::endSubRun(SubRun&, ProcessingFrame const&)
  {}

} // namespace art
