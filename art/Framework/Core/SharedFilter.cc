#include "art/Framework/Core/SharedFilter.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  SharedFilter::workerType() const
  {
    return "WorkerT<SharedFilter>";
  }

  void
  SharedFilter::setupQueues()
  {
    createQueues();
  }

  bool
  SharedFilter::filterWithScheduleID(Event& e, ScheduleID const sid)
  {
    return filter(e, sid);
  }

} // namespace art
