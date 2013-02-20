#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "art/Utilities/quiet_unit_test.hpp"
using namespace boost::unit_test;

#include "tech-testbed/EventPrincipalQueue.hh"
#include "tech-testbed/ScheduleBroker.hh"
#include "tech-testbed/ScheduleQueue.hh"

#include "tbb/task_scheduler_init.h"

#include <cstdlib>
#include <cstring>

class TaskTestbed {
public:
  TaskTestbed();
  void operator() (size_t nSchedules,
                     size_t nEvents);

private:
  tbb::task_scheduler_init tbbManager_;
  demo::WaitingTaskList pTasks_;
};

// Actual working function
void exec_taskTestbed() {
  TaskTestbed work;
  work(10, 201);
  work(15, 704);
}

bool
init_unit_test_suite()
{
  framework::master_test_suite().add(BOOST_TEST_CASE(&exec_taskTestbed));
  return true;
}

int main(int argc, char *argv[]) {
  char const *cmp = "-c";
  char to[] = "--config";
  for (int i = 0;
       i < argc;
       ++i) {
    if (strncmp(cmp, argv[i], 2) == 0) {
      argv[i] = to;
      break; // Done.
    }
  }
  return unit_test_main(&init_unit_test_suite, argc, argv);
}


TaskTestbed::TaskTestbed()
:
  tbbManager_(),
  pTasks_()
{
}

void
TaskTestbed::
operator() (size_t nSchedules,
            size_t nEvents)
{
  // Prepare the waiting task list for operation (in case this isn't the
  // first time we've been called).
  pTasks_.reset();

  // Queues.
  demo::ScheduleQueue sQ;
  demo::EventPrincipalQueue epQ;

  // Dummy task (never spawned) to handle all the others as children.
  std::shared_ptr<tbb::task> topTask
  { new (tbb::task::allocate_root()) tbb::empty_task {},
      [](tbb::task* iTask){tbb::task::destroy(*iTask);}
  };
  topTask->set_ref_count(2);

  // Primary broker task which will spawn schedule tasks.
  demo::ScheduleBroker * sb { new (topTask->allocate_child())
      demo::ScheduleBroker(epQ, sQ, topTask.get(), pTasks_)
  };

  // Add ScheduleBroker task.
  pTasks_.add(sb);

  // Load scheduleQ.
  std::vector<demo::Schedule> schedules;
  for (size_t i { 0 }; i < nSchedules; ++i) {
    schedules.emplace_back(i);
    sQ.push(cet::exempt_ptr<demo::Schedule>(&schedules.back()));
  }

  pTasks_.doneWaiting(); // Start processing events.

  // Some EventPrincipals to process.
  std::vector<demo::EventPrincipal> principals;
  principals.reserve(nEvents);
  for (size_t i { 0 }; i < nEvents; ++i) {
    principals.emplace_back();
    epQ.push(cet::exempt_ptr<demo::EventPrincipal>(&principals.back()));
    usleep(10000);
  }

  __sync_synchronize();

  // Done for now. Tell the broker to finish when it's done with
  // outstanding events.
  sb->drain();

  // Wait for all tasks to complete.
  topTask->wait_for_all();

  size_t total = 0;
  for (auto const & sched : schedules) {
    total += sched.eventsProcessed();
  }

  BOOST_REQUIRE_EQUAL(nEvents, total);

  return;
}

// Local Variables:
// mode: c++
// End:
