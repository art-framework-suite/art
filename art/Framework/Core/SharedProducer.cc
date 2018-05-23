#include "art/Framework/Core/SharedProducer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  SharedProducer::workerType() const
  {
    return "WorkerT<SharedProducer>";
  }

  void
  SharedProducer::setupQueues()
  {
    createQueues();
  }

  void
  SharedProducer::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                     Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  SharedProducer::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                      Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  SharedProducer::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                       Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  SharedProducer::respondToCloseOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  SharedProducer::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  SharedProducer::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  void
  SharedProducer::beginRunWithServices(Run& r, Services const& services)
  {
    beginRun(r, services);
  }

  void
  SharedProducer::endRunWithServices(Run& r, Services const& services)
  {
    endRun(r, services);
  }

  void
  SharedProducer::beginSubRunWithServices(SubRun& sr, Services const& services)
  {
    beginSubRun(sr, services);
  }

  void
  SharedProducer::endSubRunWithServices(SubRun& sr, Services const& services)
  {
    endSubRun(sr, services);
  }

  void
  SharedProducer::produceWithServices(Event& e, Services const& services)
  {
    produce(e, services);
  }

  // Default implementations
  void
  SharedProducer::beginJob(Services const&)
  {}

  void
  SharedProducer::endJob(Services const&)
  {}

  void
  SharedProducer::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedProducer::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedProducer::respondToOpenOutputFiles(FileBlock const&, Services const&)
  {}

  void
  SharedProducer::respondToCloseOutputFiles(FileBlock const&, Services const&)
  {}

  void
  SharedProducer::beginRun(Run&, Services const&)
  {}

  void
  SharedProducer::endRun(Run&, Services const&)
  {}

  void
  SharedProducer::beginSubRun(SubRun&, Services const&)
  {}

  void
  SharedProducer::endSubRun(SubRun&, Services const&)
  {}

} // namespace art
