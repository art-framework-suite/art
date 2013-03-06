#ifndef tech_testbed_ScheduleTask_hh
#define tech_testbed_ScheduleTask_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/EventPrincipal.hh"
#include "tech-testbed/EventQueue.hh"
#include "tech-testbed/Schedule.hh"
#include "tech-testbed/SerialTaskQueue.hh"

#include "tbb/task.h"

#include <memory>

namespace demo {
  class ScheduleTask;
}

class demo::ScheduleTask : public tbb::task {
public:
ScheduleTask(std::shared_ptr<EventPrincipal> && ep,
             cet::exempt_ptr<Schedule> sched,
             task * topTask,
             SerialTaskQueue & sQ,
             EventQueue & eQ);
  task * execute() final override;
private:
  std::shared_ptr<EventPrincipal> ep_;
  cet::exempt_ptr<Schedule> const sched_;
  tbb::task * topTask_;
  SerialTaskQueue & sQ_;
  EventQueue & eQ_;
};
#endif /* tech_testbed_ScheduleTask_hh */
