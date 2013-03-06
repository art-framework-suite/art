cet_enable_asserts()

include(CetTest)

art_make_library(SOURCE
  Schedule.cc
  ScheduleTask.cc
  SerialTaskQueue.cc
  make_reader.cc
  LIBRARIES
  ${TBB}
  -lrt
  )

# Test code from Chris J.
cet_test(SerialTaskQueue_test USE_BOOST_UNIT
  LIBRARIES tech-testbed
  )

cet_test(taskTestbed USE_BOOST_UNIT NO_AUTO
  LIBRARIES tech-testbed
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
