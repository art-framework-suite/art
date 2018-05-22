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

  bool
  ReplicatedFilter::filterWithScheduleID(Event& e, ScheduleID)
  {
    return filter(e);
  }

} // namespace art
