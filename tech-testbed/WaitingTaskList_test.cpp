//
//  WaitingTaskList_test.cpp
//  DispatchProcessingDemo
//
//  Created by Chris Jones on 9/27/11.
//  Copyright 2011 FNAL. All rights reserved.
//

#define BOOST_TEST_MODULE ( SerialTaskQueue_test )
#include "boost/test/auto_unit_test.hpp"

#include "WaitingTaskList.h"

#include "tbb/task.h"
#include "boost/shared_ptr.hpp"

#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#define CXX_THREAD_AVAILABLE
#endif

namespace  {
   class TestCalledTask : public tbb::task {
   public:
      TestCalledTask(std::atomic<bool>& iCalled): m_called(iCalled) {}

      tbb::task* execute() {
         m_called = true;
         return nullptr;
      }

   private:
      std::atomic<bool>& m_called;
   };

   class TestValueSetTask : public tbb::task {
   public:
      TestValueSetTask(std::atomic<bool>& iValue): m_value(iValue) {}
         tbb::task* execute() {
            BOOST_REQUIRE(m_value);
            return nullptr;
         }

      private:
         std::atomic<bool>& m_value;
   };

}

BOOST_AUTO_TEST_SUITE(WaitingTaskList_test)

BOOST_AUTO_TEST_CASE(addThenDone)
{
   std::atomic<bool> called{false};

   demo::WaitingTaskList waitList;
   {
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
                                            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(2);
      //NOTE: allocate_child does NOT increment the ref_count of waitTask!
      tbb::task* t = new (waitTask->allocate_child()) TestCalledTask{called};

      waitList.add(t);

      usleep(10);
      __sync_synchronize();
      BOOST_REQUIRE(false==called);

      waitList.doneWaiting();
      waitTask->wait_for_all();
      __sync_synchronize();
      BOOST_REQUIRE(true==called);
   }

   waitList.reset();
   called = false;

   {
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
                                            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(2);

      tbb::task* t = new (waitTask->allocate_child()) TestCalledTask{called};

      waitList.add(t);

      usleep(10);
      BOOST_REQUIRE(false==called);

      waitList.doneWaiting();
      waitTask->wait_for_all();
      BOOST_REQUIRE(true==called);
   }
}

BOOST_AUTO_TEST_CASE(doneThenAdd)
{
   std::atomic<bool> called{false};
   demo::WaitingTaskList waitList;
   {
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
                                            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(2);

      tbb::task* t = new (waitTask->allocate_child()) TestCalledTask{called};

      waitList.doneWaiting();

      waitList.add(t);
      waitTask->wait_for_all();
      BOOST_REQUIRE(true==called);
   }
}

namespace {
#if defined(CXX_THREAD_AVAILABLE)
   void join_thread(std::thread* iThread){
      if(iThread->joinable()){iThread->join();}
   }
#endif
}

BOOST_AUTO_TEST_CASE(stressTest)
{
#if defined(CXX_THREAD_AVAILABLE)
   std::atomic<bool> called{false};
   demo::WaitingTaskList waitList;

   unsigned int index = 1000;
   const unsigned int nTasks = 10000;
   while(0 != --index) {
      called = false;
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
                                            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(3);
      tbb::task* pWaitTask=waitTask.get();

      {
         std::thread makeTasksThread([&waitList,pWaitTask,&called]{
            for(unsigned int i = 0; i<nTasks;++i) {
               auto t = new (tbb::task::allocate_additional_child_of(*pWaitTask)) TestCalledTask{called};
               waitList.add(t);
            }

            pWaitTask->decrement_ref_count();
            });
         boost::shared_ptr<std::thread>(&makeTasksThread,join_thread);

         std::thread doneWaitThread([&waitList,&called,pWaitTask]{
            called=true;
            waitList.doneWaiting();
            pWaitTask->decrement_ref_count();
            });
         boost::shared_ptr<std::thread>(&doneWaitThread,join_thread);
      }
      waitTask->wait_for_all();
   }
#endif
}

BOOST_AUTO_TEST_SUITE_END()
