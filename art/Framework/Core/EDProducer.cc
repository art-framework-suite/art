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
  EDProducer::produceWithScheduleID(Event& e, ScheduleID const sid)
  {
    ScheduleIDSentry sentry{*this, sid};
    produce(e);
  }

} // namespace art
