#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "art/Utilities/quiet_unit_test.hpp"
using namespace boost::unit_test;

#include "tbb/task_scheduler_init.h"

#include <cstdlib>
#include <cstring>

class TaskTestbed {
public:
  TaskTestbed();
  bool operator() ();

private:
  tbb::task_scheduler_init tbbManager_;
};

// Actual working function
void exec_taskTestbed() {
  TaskTestbed work;
  BOOST_REQUIRE(work());
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

bool
TaskTestbed::
operator() ()
{
  return true;
}

// Local Variables:
// mode: c++
// End:
