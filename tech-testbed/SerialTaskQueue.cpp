/*
 *  SerialTaskQueue.cpp
 *  TBBProcessingDemo
 *
 *  Created by Chris Jones on 9/17/09.
 *  Copyright 2009 FNAL. All rights reserved.
 *
 */

#include "SerialTaskQueue.h"

using namespace demo;

bool
SerialTaskQueue::resume() {
  if(0==--m_pauseCount) {
    tbb::task* t = pickNextTask();
    if(0 != t) {
      tbb::task::spawn(*t);
    }
    return true;
  }
  return false;
}

void
SerialTaskQueue::pushTask(SerialTaskQueue::TaskBase* iTask) {
  tbb::task* t = pushAndGetNextTask(iTask);
  if(0!=t) {
    tbb::task::spawn(*t);      
  }
}

tbb::task* 
SerialTaskQueue::pushAndGetNextTask(TaskBase* iTask) {
  tbb::task* returnValue{0};
  if(0!=iTask) {
    m_tasks.push(iTask);
    returnValue = pickNextTask();
  }
  return returnValue;
}


tbb::task*
SerialTaskQueue::finishedTask() {
  m_taskChosen.clear();
  return pickNextTask();
}

SerialTaskQueue::TaskBase*
SerialTaskQueue::pickNextTask() {
  
  if(0 == m_pauseCount and not m_taskChosen.test_and_set()) {
    TaskBase* t=0;
    if(m_tasks.try_pop(t)) {
      return t;
    }
    //no task was actually pulled
    m_taskChosen.clear();
    
    //was a new entry added after we called 'try_pop' but before we did the clear?
    if(not m_tasks.empty() and not m_taskChosen.test_and_set()) {
      TaskBase* t=0;
      if(m_tasks.try_pop(t)) {
        return t;
      }
      //no task was still pulled since a different thread beat us to it
      m_taskChosen.clear();
      
    }
  }
  return 0;
}
