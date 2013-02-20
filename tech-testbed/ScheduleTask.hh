#ifndef tech_testbed_ScheduleTask_hh
#define tech_testbed_ScheduleTask_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/EventPrincipal.hh"
#include "tech-testbed/Schedule.hh"
#include "tech-testbed/ScheduleQueue.hh"

#include "tbb/task.h"

namespace demo {
  class ScheduleTask;
}

class demo::ScheduleTask : public tbb::task {
public:
  ScheduleTask(cet::exempt_ptr<EventPrincipal> const & ep,
               cet::exempt_ptr<Schedule> const & sched,
               ScheduleQueue & sQ);
  task * execute() final override;
private:
  cet::exempt_ptr<EventPrincipal> const & ep_;
  cet::exempt_ptr<Schedule> const & sched_;
  ScheduleQueue & sQ_;
};
#endif /* tech_testbed_ScheduleTask_hh */
