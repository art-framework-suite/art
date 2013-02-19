//
//  SerialTaskQueue_test.cpp
//  DispatchProcessingDemo
//
//  Created by Chris Jones on 9/27/11.
//  Copyright 2011 FNAL. All rights reserved.
//

#define BOOST_TEST_MODULE ( SerialTaskQueue_test )
#include "boost/test/auto_unit_test.hpp"

#include "SerialTaskQueue.h"

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

namespace {
  void testPush()
  {
    std::atomic<unsigned int> count{0};

    demo::SerialTaskQueue queue;
    {
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
          [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(1+3);
      tbb::task* pWaitTask = waitTask.get();

      queue.push([&count,pWaitTask]{
          BOOST_REQUIRE(count++ == 0);
          usleep(10);
          pWaitTask->decrement_ref_count();
        });

      queue.push([&count,pWaitTask]{
          BOOST_REQUIRE(count++ == 1);
          usleep(10);
          pWaitTask->decrement_ref_count();
        });

      queue.push([&count,pWaitTask]{
          BOOST_REQUIRE(count++ == 2);
          usleep(10);
          pWaitTask->decrement_ref_count();
        });

      waitTask->wait_for_all();
      BOOST_REQUIRE(count==3);
    }
  }

  void testPushAndWait()
  {
    std::atomic<unsigned int> count{0};

    demo::SerialTaskQueue queue;
    {
      queue.push([&count]{
          BOOST_REQUIRE(count++ == 0);
          usleep(10);
        });

      queue.push([&count]{
          BOOST_REQUIRE(count++ == 1);
          usleep(10);
        });

      queue.pushAndWait([&count]{
          BOOST_REQUIRE(count++ == 2);
          usleep(10);
        });

      BOOST_REQUIRE(count==3);
    }
  }

  void testPause()
  {
    std::atomic<unsigned int> count{0};

    demo::SerialTaskQueue queue;
    {
      queue.pause();
      {
        boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
        waitTask->set_ref_count(1+1);
        tbb::task* pWaitTask = waitTask.get();

        queue.push([&count,pWaitTask]{
            BOOST_REQUIRE(count++ == 0);
            pWaitTask->decrement_ref_count();
          });
        usleep(100);
        BOOST_REQUIRE(0==count);
        queue.resume();
        waitTask->wait_for_all();
        BOOST_REQUIRE(count==1);
      }

      {
        boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
            [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
        waitTask->set_ref_count(1+3);
        tbb::task* pWaitTask = waitTask.get();

        queue.push([&count,&queue,pWaitTask]{
            queue.pause();
            BOOST_REQUIRE(count++ == 1);
            pWaitTask->decrement_ref_count();
          });
        queue.push([&count,&queue,pWaitTask]{
            BOOST_REQUIRE(count++ == 2);
            pWaitTask->decrement_ref_count();
          });
        queue.push([&count,&queue,pWaitTask]{
            BOOST_REQUIRE(count++ == 3);
            pWaitTask->decrement_ref_count();
          });
        usleep(100);
        BOOST_REQUIRE(2==count);
        queue.resume();
        waitTask->wait_for_all();
        BOOST_REQUIRE(count==4);
      }
    }
  }

#if defined(CXX_THREAD_AVAILABLE)
   void join_thread(std::thread* iThread){
      if(iThread->joinable()){iThread->join();}
   }
#endif

  void stressTest()
  {
#if defined(CXX_THREAD_AVAILABLE)
    demo::SerialTaskQueue queue;

    unsigned int index = 100;
    const unsigned int nTasks = 1000;
    while(0 != --index) {
      boost::shared_ptr<tbb::task> waitTask{new (tbb::task::allocate_root()) tbb::empty_task{},
          [](tbb::task* iTask){tbb::task::destroy(*iTask);} };
      waitTask->set_ref_count(3);
      tbb::task* pWaitTask=waitTask.get();
      std::atomic<unsigned int> count{0};

      std::atomic<bool> waitToStart{true};
      {
        std::thread pushThread([&queue,&waitToStart,pWaitTask,&count]{
            //gcc 4.7 doesn't preserve the 'atomic' nature of waitToStart in the loop
            while(waitToStart.load()) {__sync_synchronize();};
            for(unsigned int i = 0; i<nTasks;++i) {
              pWaitTask->increment_ref_count();
              queue.push([i,&count,pWaitTask] {
                  ++count;
                  pWaitTask->decrement_ref_count();
                });
            }

            pWaitTask->decrement_ref_count();
          });

        waitToStart=false;
        for(unsigned int i=0; i<nTasks;++i) {
          pWaitTask->increment_ref_count();
          queue.push([i,&count,pWaitTask] {
              ++count;
              pWaitTask->decrement_ref_count();
            });
        }
        pWaitTask->decrement_ref_count();
        boost::shared_ptr<std::thread>(&pushThread,join_thread);
      }
      waitTask->wait_for_all();

      BOOST_REQUIRE(2*nTasks==count);
    }
#endif
  }

}



BOOST_AUTO_TEST_SUITE (SerialTaskQueue_test)

BOOST_AUTO_TEST_CASE(testPush)
{
  testPush();
}

BOOST_AUTO_TEST_CASE(testPushAndWait)
{
  testPushAndWait();
}

BOOST_AUTO_TEST_CASE(testPause)
{
  testPause();
}

BOOST_AUTO_TEST_CASE(stressTest)
{
  stressTest();
}

BOOST_AUTO_TEST_SUITE_END()
