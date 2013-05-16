#include "art/Framework/Services/Registry/get_scheduleID.h"

#include "art/Framework/Services/Registry/detail/ScheduleTask.h"
#include "art/Utilities/Exception.h"

art::ScheduleID
art::get_scheduleID()
{
  tbb::task * ct = & tbb::task::self();
  detail::ScheduleTask * st;
  while (ct != nullptr &&
         (st  = dynamic_cast<detail::ScheduleTask *>(ct)) == nullptr) {
    ct = ct->parent();
  }
  if (st != nullptr) {
    return st->scheduleID();
  }
  else {
    throw Exception(errors::ScheduleExecutionFailure, "get_ScheduleID: ")
      << "Unable to ascertain current schedule ID.\n";
  }
}
