#ifndef tech_testbed_ScheduleBroker_hh
#define tech_testbed_ScheduleBroker_hh

#include "tech-testbed/EventPrincipalQueue.hh"
#include "tech-testbed/ScheduleTask.hh"
#include "tech-testbed/WaitingTaskList.h"

#include "tbb/task.h"

#include <atomic>

namespace demo {
  class ScheduleBroker;
}

class demo::ScheduleBroker : public tbb::task {
public:
  ScheduleBroker(EventPrincipalQueue & epQ,
                 ScheduleQueue & sQ,
                 tbb::task * topTask,
                 WaitingTaskList & pTasks);
  void drain();
  tbb::task * execute() final override;
private:
  std::atomic_bool draining_;
  EventPrincipalQueue & epQ_;
  ScheduleQueue & sQ_;
  tbb::task * topTask_;
  WaitingTaskList & pTasks_;
};
#endif /* tech_testbed_ScheduleBroker_hh */
