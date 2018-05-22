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

  bool
  EDFilter::filterWithScheduleID(Event& e, ScheduleID const sid)
  {
    ScheduleIDSentry sentry{*this, sid};
    return filter(e);
  }

} // namespace art
