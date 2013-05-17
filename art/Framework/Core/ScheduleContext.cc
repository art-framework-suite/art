#include "art/Framework/Core/ScheduleContext.h"

#include "art/Framework/Services/Registry/detail/ScheduleTask.h"
#include "art/Utilities/Exception.h"

#include "tbb/task.h"

art::ScheduleID
art::ScheduleContext::
currentScheduleID()
{
  if (!instance().in_context_) { return ScheduleID(); } // Not in schedule-running context.
  tbb::task * ct = & tbb::task::self();
  detail::ScheduleTask * st { nullptr };
  while (ct != nullptr &&
         (st = dynamic_cast<detail::ScheduleTask *>(ct)) == nullptr) {
    ct = ct->parent();
  }
  return (st ? st->scheduleID() : ScheduleID());
}

art::ScheduleContext::
ScheduleContext()
  :
  in_context_(false)
{
}

art::ScheduleContext &
art::ScheduleContext::instance()
{
  static ScheduleContext s_context;
  return s_context;
}

bool
art::ScheduleContext::
setContext()
{
  bool const result = in_context_;
  in_context_ = true;
  return result;
}

bool
art::ScheduleContext::
resetContext()
{
  bool const result = in_context_;
  in_context_ = false;
  return result;
}
