// Sketch of one way to do a scaling study
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/tick_count.h"

extern "C" {
#include <unistd.h>
}

int main()
{
  int n = tbb::task_scheduler_init::default_num_threads();
  for (int p = 1; p <= n; ++p) {
    // Construct task scheduler with p threads
    tbb::task_scheduler_init init(p);
    auto t0 = tbb::tick_count::now();
    // Execute parallel algorithm using task or template algorithm here.
    usleep(4005 / n);
    auto t1 = tbb::tick_count::now();
    double t = (t1 - t0).seconds();
    std::cout << "time = " << t << " with " << p << " threads." << std::endl;
    // Implicitly destroy task scheduler.
  }
  return 0;
}
