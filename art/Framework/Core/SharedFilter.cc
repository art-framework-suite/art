#include "art/Framework/Core/SharedFilter.h"
// vim: set sw=2 expandtab :

namespace art {

  void
  SharedFilter::setupQueues(detail::SharedResources const& resources)
  {
    createQueues(resources);
  }

  void
  SharedFilter::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                                ProcessingFrame const& frame)
  {
    respondToOpenInputFile(fb, frame);
  }

  void
  SharedFilter::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                                 ProcessingFrame const& frame)
  {
    respondToCloseInputFile(fb, frame);
  }

  void
  SharedFilter::respondToOpenOutputFilesWithFrame(FileBlock const& fb,
                                                  ProcessingFrame const& frame)
  {
    respondToOpenOutputFiles(fb, frame);
  }

  void
  SharedFilter::respondToCloseOutputFilesWithFrame(FileBlock const& fb,
                                                   ProcessingFrame const& frame)
  {
    respondToCloseOutputFiles(fb, frame);
  }

  void
  SharedFilter::beginJobWithFrame(ProcessingFrame const& frame)
  {
    beginJob(frame);
  }

  void
  SharedFilter::endJobWithFrame(ProcessingFrame const& frame)
  {
    endJob(frame);
  }

  bool
  SharedFilter::beginRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    beginRun(r, frame);
    return true;
  }

  bool
  SharedFilter::endRunWithFrame(Run& r, ProcessingFrame const& frame)
  {
    endRun(r, frame);
    return true;
  }

  bool
  SharedFilter::beginSubRunWithFrame(SubRun& sr, ProcessingFrame const& frame)
  {
    beginSubRun(sr, frame);
    return true;
  }

  bool
  SharedFilter::endSubRunWithFrame(SubRun& sr, ProcessingFrame const& frame)
  {
    endSubRun(sr, frame);
    return true;
  }

  bool
  SharedFilter::filterWithFrame(Event& e, ProcessingFrame const& frame)
  {
    return filter(e, frame);
  }

  // Default implementations
  void
  SharedFilter::beginJob(ProcessingFrame const&)
  {}

  void
  SharedFilter::endJob(ProcessingFrame const&)
  {}

  void
  SharedFilter::respondToOpenInputFile(FileBlock const&, ProcessingFrame const&)
  {}

  void
  SharedFilter::respondToCloseInputFile(FileBlock const&,
                                        ProcessingFrame const&)
  {}

  void
  SharedFilter::respondToOpenOutputFiles(FileBlock const&,
                                         ProcessingFrame const&)
  {}

  void
  SharedFilter::respondToCloseOutputFiles(FileBlock const&,
                                          ProcessingFrame const&)
  {}

  void
  SharedFilter::beginRun(Run&, ProcessingFrame const&)
  {}

  void
  SharedFilter::endRun(Run&, ProcessingFrame const&)
  {}

  void
  SharedFilter::beginSubRun(SubRun&, ProcessingFrame const&)
  {}

  void
  SharedFilter::endSubRun(SubRun&, ProcessingFrame const&)
  {}

  template class WorkerT<SharedFilter>;

} // namespace art
