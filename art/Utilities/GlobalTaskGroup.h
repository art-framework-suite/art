#ifndef art_Utilities_GlobalTaskGroup_h
#define art_Utilities_GlobalTaskGroup_h

#include "hep_concurrency/WaitingTask.h"
#include "tbb/global_control.h"
#include "tbb/task_group.h"

#include <exception>

namespace art {
  class GlobalTaskGroup {
  public:
    GlobalTaskGroup(unsigned n_threads, unsigned stack_size);

    template <typename T>
    void
    run(T&& t)
    {
      group_.run(std::move(t));
    }

    void may_run(hep::concurrency::WaitingTaskPtr task,
                 std::exception_ptr ex_ptr = {});

    // Get rid of this!
    tbb::task_group&
    native_group()
    {
      return group_;
    }

  private:
    tbb::global_control threadControl_;
    tbb::global_control stackSizeControl_;
    tbb::task_group group_;
  };
}

#endif /* art_Utilities_GlobalTaskGroup_h */

// Local Variables:
// mode: c++
// End:
