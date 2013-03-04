#ifndef tech_testbed_ScheduleTask_hh
#define tech_testbed_ScheduleTask_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/EventPrincipal.hh"
#include "tech-testbed/Schedule.hh"
#include "tech-testbed/SerialTaskQueue.hh"

#include "tbb/task.h"

#include <memory>

namespace demo {
  class ScheduleTask;
}

class demo::ScheduleTask : public tbb::task {
public:
ScheduleTask(std::unique_ptr<EventPrincipal> && ep,
             cet::exempt_ptr<Schedule> sched,
             task * topTask,
             SerialTaskQueue & sQ,
             size_t & evCounter);
  task * execute() final override;
private:
  std::unique_ptr<EventPrincipal> ep_;
  cet::exempt_ptr<Schedule> const sched_;
  tbb::task * topTask_;
  SerialTaskQueue & sQ_;
  size_t & evCounter_;
};
#endif /* tech_testbed_ScheduleTask_hh */
