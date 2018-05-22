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
  ReplicatedProducer::produceWithScheduleID(Event& e, ScheduleID)
  {
    produce(e);
  }

} // namespace art
