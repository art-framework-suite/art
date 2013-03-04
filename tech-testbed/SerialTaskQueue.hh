#ifndef tech_testbed_SerialTaskQueue_hh
#define tech_testbed_SerialTaskQueue_hh
////////////////////////////////////////////////////////////////////////
// SerialTaskQueue.
//
// Original: Chris Jones on 9/18/09.
//  Copyright 2009 FNAL. All rights reserved.
//
////////////////////////////////////////////////////////////////////////

#include <atomic>

#include "tbb/task.h"
#include "tbb/concurrent_queue.h"

namespace demo {
  class SerialTaskQueue;
}

class demo::SerialTaskQueue {
public:
  SerialTaskQueue(SerialTaskQueue const &) = delete;
  const SerialTaskQueue & operator = (SerialTaskQueue const &) = delete;

  SerialTaskQueue();

  bool isPaused() const;

  // Returns true if not already paused.
  bool pause();

  // Returns true if this really restarts the queue.
  bool resume();

  template <typename T>
  void
  push(T const & action);

  template <typename T>
  void
  pushAndWait(T const & action);

  template<typename T>
  tbb::task *
  pushAndGetNextTaskToRun(T const & action);

private:
  class TaskBase : public tbb::task {
    friend class SerialTaskQueue;
    TaskBase();
  protected:
    tbb::task * finishedTask();
  private:
    void setQueue(SerialTaskQueue * queue);

    SerialTaskQueue * queue_;
  };

  template <typename T>
  class QueuedTask : public TaskBase {
  public:
    QueuedTask(T const & action);
  private:
    tbb::task * execute();

    T action_;
  };

  friend class TaskBase;

  void pushTask(TaskBase *);
  tbb::task * pushAndGetNextTask(TaskBase *);
  tbb::task * finishedTask();
  //returns nullptr if a task is already being processed
  TaskBase * pickNextTask();

  void pushAndWait(tbb::empty_task * wait, TaskBase * task);

  tbb::concurrent_queue<TaskBase *> tasks_;
  std::atomic_flag taskChosen_;
  std::atomic<unsigned long> pauseCount_;
};

inline
demo::SerialTaskQueue::
SerialTaskQueue()
  :
  taskChosen_ {ATOMIC_FLAG_INIT},
pauseCount_ {0} {
}

inline
bool
demo::SerialTaskQueue::
isPaused() const
{
  return pauseCount_.load() == 0;
}

// Returns true if not already paused.
inline
bool
demo::SerialTaskQueue::
pause()
{
  return 1 == ++pauseCount_;
}

template<typename T>
void
demo::SerialTaskQueue::
push(T const & action)
{
  QueuedTask<T> * pTask {new(tbb::task::allocate_root()) QueuedTask<T>{action}};
  pTask->setQueue(this);
  pushTask(pTask);
}

template<typename T>
void
demo::SerialTaskQueue::
pushAndWait(T const & action)
{
  tbb::empty_task * waitTask {new(tbb::task::allocate_root()) tbb::empty_task};
  waitTask->set_ref_count(2);
  QueuedTask<T> * pTask {new(waitTask->allocate_child()) QueuedTask<T>{action}};
  pTask->setQueue(this);
  pushAndWait(waitTask, pTask);
}

template<typename T>
tbb::task *
demo::SerialTaskQueue::
pushAndGetNextTaskToRun(T const & action)
{
  QueuedTask<T> * pTask {new(tbb::task::allocate_root()) QueuedTask<T>{action}};
  pTask->setQueue(this);
  return pushAndGetNextTask(pTask);
}

inline
demo::SerialTaskQueue::TaskBase::
TaskBase()
  :
  queue_(0)
{
}

inline
void
demo::SerialTaskQueue::TaskBase::
setQueue(SerialTaskQueue * queue)
{
  queue_ = queue;
}

template <typename T>
inline
demo::SerialTaskQueue::QueuedTask<T>::
QueuedTask(T const & action)
  :
  action_(action)
{
}


inline
tbb::task *
demo::SerialTaskQueue::TaskBase::
finishedTask()
{
  return queue_->finishedTask();
}

template <typename T>
tbb::task *
demo::SerialTaskQueue::QueuedTask<T>::
execute()
{
  try {
    this->action_();
  }
  catch (...) {
  }
  return this->finishedTask();
}

#endif /* tech_testbed_SerialTaskQueue_hh */

// Local Variables:
// mode: c++
// End:
