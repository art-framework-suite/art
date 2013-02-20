#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "art/Utilities/quiet_unit_test.hpp"
using namespace boost::unit_test;

#include "tech-testbed/SerialTaskQueue.h"
#include "tech-testbed/make_reader.hh"

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
};

// Actual working function
void exec_taskTestbed() {
  TaskTestbed work;
  work(10, 20021);
  work(15, 70004);
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
  tbbManager_()
{
}

void
TaskTestbed::
operator() (size_t nSchedules,
            size_t nEvents)
{

  demo::SerialTaskQueue sQ;

  // Dummy task (never spawned) to handle all the others as children.
  std::shared_ptr<tbb::task> topTask
  { new (tbb::task::allocate_root()) tbb::empty_task {},
      [](tbb::task* iTask){tbb::task::destroy(*iTask);}
  };

  topTask->set_ref_count(nSchedules + 1);

  size_t evCounter = nEvents;

  std::vector<demo::Schedule> schedules;
  schedules.reserve(nSchedules);
  for (size_t i { 0 }; i < nSchedules; ++i) {
    schedules.emplace_back(i);
    sQ.push(demo::make_reader(cet::exempt_ptr<demo::Schedule>(&schedules.back()),
                              topTask.get(),
                              sQ,
                              evCounter));
  }

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
