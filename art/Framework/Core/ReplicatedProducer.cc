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
  ReplicatedProducer::respondToOpenInputFileWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  ReplicatedProducer::respondToCloseInputFileWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  ReplicatedProducer::respondToOpenOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  ReplicatedProducer::respondToCloseOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  ReplicatedProducer::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  ReplicatedProducer::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  void
  ReplicatedProducer::beginRunWithServices(Run& r, Services const& services)
  {
    beginRun(r, services);
  }

  void
  ReplicatedProducer::endRunWithServices(Run& r, Services const& services)
  {
    endRun(r, services);
  }

  void
  ReplicatedProducer::beginSubRunWithServices(SubRun& sr,
                                              Services const& services)
  {
    beginSubRun(sr, services);
  }

  void
  ReplicatedProducer::endSubRunWithServices(SubRun& sr,
                                            Services const& services)
  {
    endSubRun(sr, services);
  }

  void
  ReplicatedProducer::produceWithServices(Event& e, Services const& services)
  {
    produce(e, services);
  }

  // Default implementations
  void
  ReplicatedProducer::beginJob(Services const&)
  {}

  void
  ReplicatedProducer::endJob(Services const&)
  {}

  void
  ReplicatedProducer::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedProducer::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedProducer::respondToOpenOutputFiles(FileBlock const&,
                                               Services const&)
  {}

  void
  ReplicatedProducer::respondToCloseOutputFiles(FileBlock const&,
                                                Services const&)
  {}

  void
  ReplicatedProducer::beginRun(Run&, Services const&)
  {}

  void
  ReplicatedProducer::endRun(Run&, Services const&)
  {}

  void
  ReplicatedProducer::beginSubRun(SubRun&, Services const&)
  {}

  void
  ReplicatedProducer::endSubRun(SubRun&, Services const&)
  {}

} // namespace art
