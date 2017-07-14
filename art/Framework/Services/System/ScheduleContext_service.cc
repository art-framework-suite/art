#include "art/Framework/Services/System/ScheduleContext.h"

#include "art/Framework/Core/detail/ScheduleTask.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/ProvideFilePathMacro.h"

#include "tbb/task.h"

art::ScheduleID
art::ScheduleContext::currentScheduleID() const
{
  if (!in_context_.load()) { return ScheduleID{}; } // Not in schedule-running context.
  tbb::task* ct = &tbb::task::self();
  detail::ScheduleTask* st {nullptr};
  while (ct != nullptr &&
         (st = dynamic_cast<detail::ScheduleTask*>(ct)) == nullptr) {
    ct = ct->parent();
  }
  return (st ? st->scheduleID() : ScheduleID());
}

// ===============================
CET_PROVIDE_FILE_PATH()
// ===============================
