#include "art/Framework/Core/EDProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"

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
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
  }

  void
  EDProducer::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                 Services const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDProducer::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                  Services const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDProducer::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                   Services const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDProducer::respondToCloseOutputFilesWithServices(FileBlock const& fb,
                                                    Services const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDProducer::beginJobWithServices(Services const&)
  {
    beginJob();
  }

  void
  EDProducer::endJobWithServices(Services const&)
  {
    endJob();
  }

  void
  EDProducer::beginRunWithServices(Run& r, Services const&)
  {
    beginRun(r);
  }

  void
  EDProducer::endRunWithServices(Run& r, Services const&)
  {
    endRun(r);
  }

  void
  EDProducer::beginSubRunWithServices(SubRun& sr, Services const&)
  {
    beginSubRun(sr);
  }

  void
  EDProducer::endSubRunWithServices(SubRun& sr, Services const&)
  {
    endSubRun(sr);
  }

  void
  EDProducer::produceWithServices(Event& e, Services const& services)
  {
    ScheduleIDSentry sentry{*this, services.scheduleID()};
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
