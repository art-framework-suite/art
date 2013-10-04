#include "tbb/concurrent_queue.h"
#include "tbb/task_scheduler_init.h"

#include <algorithm>
#include <iostream>
#include <vector>

int main()
{
  tbb::task_scheduler_init theTBB;

  std::vector<int> initVals(10);
  std::iota(initVals.begin(), initVals.end(), 0);
  tbb::concurrent_bounded_queue<int> bq(initVals.cbegin(), initVals.cend());
  int i;
  while (!bq.empty())
  {
    bq.pop(i);
    std::cout << i << std::endl;
  }
}
