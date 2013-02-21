/*
 *  TaskQueueBase.h
 *  TBBProcessingDemo
 *
 *  Created by Chris Jones on 9/18/09.
 *  Copyright 2009 FNAL. All rights reserved.
 *
 */
#ifndef tech_testbed_TaskQueueBase_h
#define tech_testbed_TaskQueueBase_h

#include "tbb/task.h"
namespace demo {
   class TaskQueueBase {
   protected:
      class TaskBase : public tbb::task {
         friend class TaskQueueBase;
         TaskBase(): m_queue(0) {}

      protected:
         tbb::task* finishedTask();
      private:
         void setQueue(TaskQueueBase* iQueue) { m_queue = iQueue;}

         TaskQueueBase* m_queue;
      };
   
      template< typename T>
      class QueuedTask : public TaskBase {
      public:
        QueuedTask( const T& iAction):
        m_action(iAction) {}

      private:
        tbb::task* execute();

        T m_action;
      };
      
   public:
        TaskQueueBase()
        {  }

        virtual ~TaskQueueBase() {}

        template<typename T>
        void push(const T& iAction) {
           QueuedTask<T>* pTask{ new (tbb::task::allocate_root()) QueuedTask<T>{iAction} };
           pTask->setQueue(this);           
           pushTask(pTask);
        }
        
        template<typename T>
        void pushAndWait(const T& iAction) {
           tbb::empty_task* waitTask = new (tbb::task::allocate_root()) tbb::empty_task;
           waitTask->set_ref_count(2);
           QueuedTask<T>* pTask{ new (waitTask->allocate_child()) QueuedTask<T>{iAction} };
           pTask->setQueue(this);           
           auto nextTask = pushAndGetNextTask(pTask);
           if(nextTask) {
              if(nextTask == pTask) {
                 //spawn and wait for all requires the task to have its parent set
                 waitTask->spawn_and_wait_for_all(*nextTask);
              } else {
                 tbb::task::spawn(*nextTask);
                 waitTask->wait_for_all();
              }
           } else {
              //a task must already be running in this queue
              waitTask->wait_for_all();              
           }
           tbb::task::destroy(*waitTask);
        }

        template<typename T>
        tbb::task* pushAndGetNextTaskToRun(const T& iAction) {
          QueuedTask<T>* pTask{ new (tbb::task::allocate_root()) QueuedTask<T>{iAction} };
          pTask->setQueue(this);           
          return pushAndGetNextTask(pTask);          
        }

      private:
        virtual void pushTask(TaskBase*)=0;
        virtual tbb::task* pushAndGetNextTask(TaskBase*)=0;
        friend class TaskBase;
        virtual tbb::task* finishedTask() = 0;
   };

   inline
   tbb::task* 
   TaskQueueBase::TaskBase::finishedTask() {return m_queue->finishedTask();}
   
   template <typename T>
   tbb::task* 
   TaskQueueBase::QueuedTask<T>::execute() {
     try {
       this->m_action();
     } catch(...) {}
     return this->finishedTask();
   }
}

#endif /* tech_testbed_TaskQueueBase_h */

// Local Variables:
// mode: c++
// End:
