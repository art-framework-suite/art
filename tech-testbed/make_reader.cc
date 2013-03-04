#include "tech-testbed/make_reader.hh"

#include "tech-testbed/ScheduleTask.hh"

#include "tbb/task.h"

#include <cassert>

std::function<void ()>
demo::
make_reader(cet::exempt_ptr<Schedule> sched,
            tbb::task * topTask,
            SerialTaskQueue & sQ,
            size_t & evCounter)
{
  assert(topTask != nullptr);

  return [sched, topTask, &sQ, &evCounter]() {
    if (evCounter == 0) {
      topTask->decrement_ref_count();
    }
    else {
      --evCounter;
      tbb::task::spawn(*(new (topTask->allocate_root())
                         ScheduleTask(std::unique_ptr<EventPrincipal>(new EventPrincipal { evCounter }),
                                      sched,
                                      topTask,
                                      sQ,
                                      evCounter)));
    }
  };
}
