simple_plugin(ThrowingAnalyzer "module" NO_INSTALL)
simple_plugin(ThrowingProducer "module" NO_INSTALL)
simple_plugin(DoNothingInput   "source" NO_INSTALL)

cet_test(EP_throwing_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_01.fcl
  DATAFILES
  fcl/test_01.fcl
  )

foreach(num RANGE 2 4)
  cet_test(EP_throwing_0${num}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c test_0${num}.fcl
    DATAFILES
    fcl/test_0${num}.fcl
    TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION "Throw from c'tor."
    )
endforeach()

cet_script(graceful_shutdown_t.sh NO_INSTALL)

cet_test(graceful_shutdown_from_exception_t HANDBUILT
  TEST_EXEC graceful_shutdown_t.sh
  TEST_ARGS -c test_graceful_shutdown.fcl
  REF "${CMAKE_CURRENT_SOURCE_DIR}/graceful_shutdown_from_exception_t-ref.txt"
  DATAFILES
  fcl/test_graceful_shutdown.fcl
  )
