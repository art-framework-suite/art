#include "art/Framework/Core/ReplicatedFilter.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  ReplicatedFilter::workerType() const
  {
    return "WorkerT<ReplicatedFilter>";
  }

  void
  ReplicatedFilter::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedFilter::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                       Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  ReplicatedFilter::respondToCloseInputFileWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  ReplicatedFilter::respondToOpenOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  ReplicatedFilter::respondToCloseOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  ReplicatedFilter::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  ReplicatedFilter::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  bool
  ReplicatedFilter::beginRunWithServices(Run& r, Services const& services)
  {
    return beginRun(r, services);
  }

  bool
  ReplicatedFilter::endRunWithServices(Run& r, Services const& services)
  {
    return endRun(r, services);
  }

  bool
  ReplicatedFilter::beginSubRunWithServices(SubRun& sr,
                                            Services const& services)
  {
    return beginSubRun(sr, services);
  }

  bool
  ReplicatedFilter::endSubRunWithServices(SubRun& sr, Services const& services)
  {
    return endSubRun(sr, services);
  }

  bool
  ReplicatedFilter::filterWithServices(Event& e, Services const& services)
  {
    return filter(e, services);
  }

  // Default implementations
  void
  ReplicatedFilter::beginJob(Services const&)
  {}

  void
  ReplicatedFilter::endJob(Services const&)
  {}

  void
  ReplicatedFilter::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedFilter::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedFilter::respondToOpenOutputFiles(FileBlock const&, Services const&)
  {}

  void
  ReplicatedFilter::respondToCloseOutputFiles(FileBlock const&, Services const&)
  {}

  bool
  ReplicatedFilter::beginRun(Run const&, Services const&)
  {
    return true;
  }

  bool
  ReplicatedFilter::endRun(Run const&, Services const&)
  {
    return true;
  }

  bool
  ReplicatedFilter::beginSubRun(SubRun const&, Services const&)
  {
    return true;
  }

  bool
  ReplicatedFilter::endSubRun(SubRun const&, Services const&)
  {
    return true;
  }

} // namespace art
