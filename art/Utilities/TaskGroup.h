#ifndef art_Utilities_TaskGroup_h
#define art_Utilities_TaskGroup_h

// This is temporary solution for now.  N.B. the FPU must be inherited
// after the TBB scheduler is initialized.  The tbb::task_group
// construction must therefore happen after that initialization.  This
// is another reason why having this be a static is not the best
// option.

#include "hep_concurrency/WaitingTask.h"
#include "tbb/task_group.h"

#include <mutex>

namespace art {

  class TaskGroup {
  public:
    static tbb::task_group&
    get()
    {
      std::lock_guard lock{m_};
      if (not instance_)
        instance_ = new tbb::task_group;
      return *instance_;
    }

    static void
    run(hep::concurrency::WaitingTaskPtr task, std::exception_ptr ex_ptr = {})
    {
      if (task->decrement_done_count() == 0u) {
        get().run([t = std::move(task), ex_ptr] {
          t->dependentTaskFailed(ex_ptr);
          (*t)();
        });
      }
    }

    static void
    shutdown()
    {
      std::lock_guard lock{m_};
      instance_->wait();
      delete instance_;
      instance_ = nullptr;
    }

  private:
    static tbb::task_group* instance_;
    static std::mutex m_;
  };
}

#endif /* art_Utilities_TaskGroup_h */

// Local Variables:
// mode: c++
// End:
