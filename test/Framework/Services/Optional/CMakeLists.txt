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

simple_plugin(LocalServiceTest "service" NO_INSTALL)
simple_plugin(LocalServiceTestAnalyzer "module" NO_INSTALL
  test_Framework_Services_Optional_LocalServiceTest_service)

simple_plugin(LocalServiceTestInterfaceImpl "service" NO_INSTALL)

cet_test(LocalServiceTest_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c LocalServiceTest_t.fcl
  DATAFILES fcl/LocalServiceTest_t.fcl
  )
