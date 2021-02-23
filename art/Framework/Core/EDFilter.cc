#include "art/Framework/Core/EDFilter.h"
// vim: set sw=2 expandtab :

namespace art {

  std::string
  EDFilter::workerType() const
  {
    return "WorkerT<EDFilter>";
  }

  void
  EDFilter::setupQueues(detail::SharedResources const& resources)
  {
    createQueues(resources);
  }

  void
  EDFilter::respondToOpenInputFileWithFrame(FileBlock const& fb,
                                            ProcessingFrame const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDFilter::respondToCloseInputFileWithFrame(FileBlock const& fb,
                                             ProcessingFrame const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDFilter::respondToOpenOutputFilesWithFrame(FileBlock const& fb,
                                              ProcessingFrame const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDFilter::respondToCloseOutputFilesWithFrame(FileBlock const& fb,
                                               ProcessingFrame const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDFilter::beginJobWithFrame(ProcessingFrame const&)
  {
    beginJob();
  }

  void
  EDFilter::endJobWithFrame(ProcessingFrame const&)
  {
    endJob();
  }

  bool
  EDFilter::beginRunWithFrame(Run& r, ProcessingFrame const&)
  {
    return beginRun(r);
  }

  bool
  EDFilter::endRunWithFrame(Run& r, ProcessingFrame const&)
  {
    return endRun(r);
  }

  bool
  EDFilter::beginSubRunWithFrame(SubRun& sr, ProcessingFrame const&)
  {
    return beginSubRun(sr);
  }

  bool
  EDFilter::endSubRunWithFrame(SubRun& sr, ProcessingFrame const&)
  {
    return endSubRun(sr);
  }

  bool
  EDFilter::filterWithFrame(Event& e, ProcessingFrame const& frame)
  {
    ScheduleIDSentry sentry{*this, frame.scheduleID()};
    return filter(e);
  }

  // Default implementations
  void
  EDFilter::beginJob()
  {}

  void
  EDFilter::endJob()
  {}

  void
  EDFilter::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDFilter::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDFilter::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDFilter::respondToCloseOutputFiles(FileBlock const&)
  {}

  bool
  EDFilter::beginRun(Run&)
  {
    return true;
  }

  bool
  EDFilter::endRun(Run&)
  {
    return true;
  }

  bool
  EDFilter::beginSubRun(SubRun&)
  {
    return true;
  }

  bool
  EDFilter::endSubRun(SubRun&)
  {
    return true;
  }

} // namespace art
