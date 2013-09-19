#include "art/Framework/Services/System/ScheduleContext.h"

#include "art/Utilities/ScheduleTask.h"
#include "art/Utilities/Exception.h"

#include "tbb/task.h"

art::ScheduleContext::
ScheduleContext()
  :
  in_context_(false),
  inContextNoTaskSID_()
{
}

art::ScheduleContext::
ScheduleContext(SIDOneReturn)
  :
  in_context_(false),
  inContextNoTaskSID_(ScheduleID::first())
{
}

art::ScheduleID
art::ScheduleContext::
currentScheduleID()
{
  if (!in_context_.load()) { return ScheduleID(); } // Not in schedule-running context.
  tbb::task * ct = & tbb::task::self();
  detail::ScheduleTask * st { nullptr };
  while (ct != nullptr &&
         (st = dynamic_cast<detail::ScheduleTask *>(ct)) == nullptr) {
    ct = ct->parent();
  }
  return (st ? st->scheduleID() : inContextNoTaskSID_);
}
