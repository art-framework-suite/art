add_subdirectory( detail )

simple_plugin(MyService "service" NO_INSTALL)

simple_plugin(MyServiceUsingIface "service" NO_INSTALL
  art_Framework_Services_Optional_TrivialFileDelivery_service)

simple_plugin(MyServiceUser "module" NO_INSTALL)

cet_test(MyService_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MyService_t.fcl
  DATAFILES fcl/MyService_t.fcl
  )

cet_test(MyServiceUsingIface_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c MyServiceUsingIface_t.fcl
  DATAFILES fcl/MyServiceUsingIface_t.fcl
  )

simple_plugin(PSTest "service" NO_INSTALL)
simple_plugin(PSTestAnalyzer "module" NO_INSTALL
  art_test_Framework_Services_Optional_PSTest_service)

simple_plugin(PSTestInterfaceImpl "service" NO_INSTALL)

cet_test(PSTest_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c PSTest_t.fcl
  DATAFILES fcl/PSTest_t.fcl
  )
