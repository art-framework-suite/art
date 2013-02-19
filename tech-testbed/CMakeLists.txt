cet_enable_asserts()

include(CetTest)

art_make_library(SOURCE
  SerialTaskQueue.cpp
  WaitingTaskList.cpp
  LIBRARIES
  ${TBB}
  )

# Test code from Chris J.
cet_test(SerialTaskQueue_test USE_BOOST_UNIT
  SOURCES SerialTaskQueue_test.cpp 
  LIBRARIES tech-testbed
  )

cet_test(WaitingTaskList_test USE_BOOST_UNIT
  SOURCES WaitingTaskList_test.cpp
  LIBRARIES tech-testbed
  )

cet_test(taskTestbed USE_BOOST_UNIT
  LIBRARIES tech-testbed
  )
