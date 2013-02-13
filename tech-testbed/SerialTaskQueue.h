/*
 *  SerialTaskQueue.h
 *  TBBProcessingDemo
 *
 *  Created by Chris Jones on 9/18/09.
 *  Copyright 2009 FNAL. All rights reserved.
 *
 */
#ifndef TBBProcessingDemo_SerialTaskQueue_h
#define TBBProcessingDemo_SerialTaskQueue_h

#include <atomic>

#include "tbb/task.h"
#include "tbb/concurrent_queue.h"

#include "TaskQueueBase.h"
namespace demo {
   class SerialTaskQueue : public TaskQueueBase {
   public:
        SerialTaskQueue():
        m_taskChosen{ATOMIC_FLAG_INIT},
        m_pauseCount{0}
        {  }

        //returns true if not already paused
        bool pause() {
           return 1 == ++m_pauseCount;
        }

        //returns true if this really restarts the queue
        bool resume();

      private:
        virtual void pushTask(TaskBase*) final override;
        virtual tbb::task* pushAndGetNextTask(TaskBase*) final override;
        virtual tbb::task* finishedTask() final override;
        //returns nullptr if a task is already being processed
        TaskBase* pickNextTask();

        tbb::concurrent_queue<TaskBase*> m_tasks;
        std::atomic_flag m_taskChosen;
        std::atomic<unsigned long> m_pauseCount;
   };
}

#endif
