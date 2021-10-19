#include "art/Utilities/GlobalTaskGroup.h"

art::GlobalTaskGroup::GlobalTaskGroup(unsigned const n_threads,
                                      unsigned const stack_size)
  : threadControl_{tbb::global_control::max_allowed_parallelism, n_threads}
  , stackSizeControl_{tbb::global_control::thread_stack_size, stack_size}
{}

void
art::GlobalTaskGroup::may_run(hep::concurrency::WaitingTaskPtr task,
                              std::exception_ptr ex_ptr)
{
  // A non-null exception pointer is always registered with the task
  // even if the task's 'done' count will not yet decrement to zero.
  // This ensures that once the task's 'done' count is zero, an
  // exception will be registered with the task when it is launched.
  if (ex_ptr) {
    task->dependentTaskFailed(ex_ptr);
  }
  if (task->decrement_done_count() == 0u) {
    group_.run([t = std::move(task)] { (*t)(); });
  }
}
