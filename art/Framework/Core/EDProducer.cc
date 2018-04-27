#include "art/Framework/Core/EDProducer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {

  string
  EDProducer::workerType() const
  {
    return "WorkerT<EDProducer>";
  }

  string
  SharedProducer::workerType() const
  {
    return "WorkerT<SharedProducer>";
  }

  string
  ReplicatedProducer::workerType() const
  {
    return "WorkerT<ReplicatedProducer>";
  }

  void
  EDProducer::setupQueues()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
  }

  void
  SharedProducer::setupQueues()
  {
    createQueues();
  }

  void
  ReplicatedProducer::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  EDProducer::produceWithScheduleID(Event& e, ScheduleID const sid)
  {
    ScheduleIDSentry sentry{*this, sid};
    produce(e);
  }

  void
  SharedProducer::produceWithScheduleID(Event& e, ScheduleID const sid)
  {
    produce(e, sid);
  }

  void
  ReplicatedProducer::produceWithScheduleID(Event& e, ScheduleID)
  {
    produce(e);
  }

} // namespace art
