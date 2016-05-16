simple_plugin(ThrowingAnalyzer "module" NO_INSTALL)
simple_plugin(ThrowingProducer "module" NO_INSTALL)
simple_plugin(DoNothingInput   "source" NO_INSTALL)

cet_test( EP_throwing_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_01.fcl
  DATAFILES
  fcl/test_01.fcl
  )

foreach(num RANGE 2 4)
  cet_test( EP_throwing_0${num}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c test_0${num}.fcl
    DATAFILES
    fcl/test_0${num}.fcl
    TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION "Throwing.*ctor"
    )
endforeach()