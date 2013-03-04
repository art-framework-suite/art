#include "tech-testbed/SerialTaskQueue.hh"

// Branch prediction indicators.
#define likely(x) (__builtin_expect(x, true))
#define unlikely(x) (__builtin_expect(x, false))

bool
demo::SerialTaskQueue::
resume()
{
  if (0 == --pauseCount_) {
    tbb::task * t = pickNextTask();
    if (0 != t) {
      tbb::task::spawn(*t);
    }
    return true;
  }
  return false;
}

void
demo::SerialTaskQueue::
pushTask(SerialTaskQueue::TaskBase * task)
{
  tbb::task * t = pushAndGetNextTask(task);
  if (0 != t) {
    tbb::task::spawn(*t);
  }
}

tbb::task *
demo::SerialTaskQueue::
pushAndGetNextTask(TaskBase * task)
{
  tbb::task * returnValue {0};
  if (0 != task) {
    tasks_.push(task);
    returnValue = pickNextTask();
  }
  return returnValue;
}

tbb::task *
demo::SerialTaskQueue::
finishedTask()
{
  taskChosen_.clear();
  return pickNextTask();
}

demo::SerialTaskQueue::TaskBase *
demo::SerialTaskQueue::
pickNextTask()
{
  if likely(0 == pauseCount_ and not taskChosen_.test_and_set()) {
    TaskBase * t = 0;
    if likely(tasks_.try_pop(t)) {
      return t;
    }
    // No task was actually pulled.
    taskChosen_.clear();
    // Was a new entry added after we called 'try_pop' but before we did the clear?
    if (not tasks_.empty() and not taskChosen_.test_and_set()) {
      TaskBase * t = 0;
      if (tasks_.try_pop(t)) {
        return t;
      }
      //No task was still pulled since a different thread beat us to it.
      taskChosen_.clear();
    }
  }
  return 0;
}

void
demo::SerialTaskQueue::
pushAndWait(tbb::empty_task * wait, TaskBase * task)
{
  auto nextTask = pushAndGetNextTask(task);
  if likely(nullptr != nextTask) {
    if likely(nextTask == task) {
      //spawn and wait for all requires the task to have its parent set
      wait->spawn_and_wait_for_all(*nextTask);
    }
    else {
      tbb::task::spawn(*nextTask);
      wait->wait_for_all();
    }
  }
  else {
    //a task must already be running in this queue
    wait->wait_for_all();
  }
  tbb::task::destroy(*wait);
}
