/*
 *  WaitingTaskList.cpp
 *  TBBProcessingDemo
 *
 *  Created by Chris Jones on 9/17/09.
 *  Copyright 2009 FNAL. All rights reserved.
 *
 */

#include "tbb/task.h"
#include "WaitingTaskList.h"
#include <iostream>
#include <cassert>
#include "ThreadingHelpers.h"

using namespace demo;


void
WaitingTaskList::reset()
{
   m_waiting = true;
   unsigned int nSeenTasks = m_lastAssignedCacheIndex;
   m_lastAssignedCacheIndex = 0;
   assert(m_head == 0);
   if (nSeenTasks > m_nodeCacheSize) {
      //need to expand so next time we don't have to do any
      // memory requests
      delete [] m_nodeCache;
      m_nodeCacheSize = nSeenTasks;
      m_nodeCache = new WaitNode[nSeenTasks];
      for(auto it = m_nodeCache, itEnd = m_nodeCache+m_nodeCacheSize; it!=itEnd; ++it) {
         it->m_fromCache=true;
      }
   }
}

WaitNode* 
WaitingTaskList::createNode(tbb::task* iTask)
{
   unsigned int index = m_lastAssignedCacheIndex++;
   
   WaitNode* returnValue;
   if( index < m_nodeCacheSize) {
      returnValue = m_nodeCache+index;
   } else {
      returnValue = new WaitNode;
      returnValue->m_fromCache=false;
   }
   returnValue->m_task = iTask;
   returnValue->m_next = returnValue;
   
   return returnValue;
}


void
WaitingTaskList::add(tbb::task* iTask) {
   iTask->increment_ref_count();
   if(!m_waiting) {
      if(0==iTask->decrement_ref_count()) {
         tbb::task::spawn(*iTask);
      }
   } else {
      WaitNode* newHead = createNode(iTask);
      WaitNode* oldHead = m_head.exchange(newHead);
      if(oldHead) {
        newHead->setNextNode(oldHead);
        //NOTE: even if 'm_waiting' changed, we don't
        // have to recheck since we beat 'announce()' in
        // the ordering of 'm_head.exchange' call so iTask
        // is guaranteed to be in the link list
      } else {
        newHead->setNextNode(0);
        if(!m_waiting) {
           //if finished waiting right before we did the
           // exchange our task will not be spawned. Also,
           // additional threads may be calling add() and swapping
           // heads and linking us to the new head.
           // It is safe to call announce from multiple threads
           announce();
        }
        
      }
   }
}

void
WaitingTaskList::announce()
{
   //Need a temporary storage since one of these tasks could
   // cause the next event to start processing which would refill 
   // this waiting list after it has been reset
   WaitNode* n = m_head.exchange(0);
   WaitNode* next;
   while(n) {
      //it is possible that 'WaitingTaskList::add' is running in a different
      // thread and we have a new 'head' but the old head has not yet been
      // attached to the new head (we identify this since 'nextNode' will return itself).
      //  In that case we have to wait until the link has been established before going on.
      while(n == (next=n->nextNode())) {
         hardware_pause();
      }
      auto t = n->m_task;
      if(0==t->decrement_ref_count()){
         tbb::task::spawn(*t);
      }
      if(!n->m_fromCache ) {
         delete n;
      }
      n=next;
   }
}

void
WaitingTaskList::doneWaiting()
{
   m_waiting=false;
   announce();
}
