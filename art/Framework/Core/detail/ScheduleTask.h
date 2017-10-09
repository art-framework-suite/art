#ifndef art_Framework_Core_detail_ScheduleTask_h
#define art_Framework_Core_detail_ScheduleTask_h
// vim: set sw=2 expandtab :

//
//  Top level schedule task for processing events.
//

#include "art/Utilities/ScheduleID.h"

#include "tbb/task.h"

namespace art {
  namespace detail {
    class ScheduleTask;
  }
} // namespace art

class art::detail::ScheduleTask : public tbb::task {
public:
  ScheduleTask(ScheduleID sid);

  ScheduleID scheduleID() const;

  tbb::task* execute() override;

private:
  ScheduleID id_;
};

inline art::detail::ScheduleTask::ScheduleTask(ScheduleID sid) : id_(sid) {}

inline art::ScheduleID
art::detail::ScheduleTask::scheduleID() const
{
  return id_;
}
#endif /* art_Framework_Core_detail_ScheduleTask_h */

// Local Variables:
// mode: c++
// End:
