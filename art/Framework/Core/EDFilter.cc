#include "art/Framework/Core/EDFilter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"

using namespace std;

namespace art {

  string
  EDFilter::workerType() const
  {
    return "WorkerT<EDFilter>";
  }

  void
  EDFilter::setupQueues()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
  }

  void
  EDFilter::respondToOpenInputFileWithServices(FileBlock const& fb,
                                               Services const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDFilter::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                Services const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDFilter::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                 Services const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDFilter::respondToCloseOutputFilesWithServices(FileBlock const& fb,
                                                  Services const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDFilter::beginJobWithServices(Services const&)
  {
    beginJob();
  }

  void
  EDFilter::endJobWithServices(Services const&)
  {
    endJob();
  }

  bool
  EDFilter::beginRunWithServices(Run& r, Services const&)
  {
    return beginRun(r);
  }

  bool
  EDFilter::endRunWithServices(Run& r, Services const&)
  {
    return endRun(r);
  }

  bool
  EDFilter::beginSubRunWithServices(SubRun& sr, Services const&)
  {
    return beginSubRun(sr);
  }

  bool
  EDFilter::endSubRunWithServices(SubRun& sr, Services const&)
  {
    return endSubRun(sr);
  }

  bool
  EDFilter::filterWithServices(Event& e, Services const& services)
  {
    ScheduleIDSentry sentry{*this, services.scheduleID()};
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
