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
  SharedProducer::produceWithScheduleID(Event& e, ScheduleID const sid)
  {
    produce(e, sid);
  }

} // namespace art
