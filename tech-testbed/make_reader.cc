#include "tech-testbed/make_reader.hh"

#include "tech-testbed/ScheduleTask.hh"

#include "tbb/task.h"

#include <cassert>

std::function<void ()>
demo::
make_reader(cet::exempt_ptr<Schedule> sched,
            tbb::task * topTask,
            SerialTaskQueue & sQ,
            EventQueue & eQ)
{
  assert(topTask != nullptr);

  return [sched, topTask, &sQ, &eQ]() {
    std::shared_ptr<EventPrincipal> ep;
    eQ.pop(ep);
    if (ep.get() == nullptr) { // Done.
      topTask->decrement_ref_count();
    }
    else {
      tbb::task::spawn(*(new (topTask->allocate_root())
                         ScheduleTask(std::move(ep),
                                      sched,
                                      topTask,
                                      sQ,
                                      eQ)));
    }
  };
}
