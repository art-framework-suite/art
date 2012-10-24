simple_plugin(MyService "service" NOINSTALL)

simple_plugin(MyServiceUsingIface "service" NOINSTALL
  art_Framework_Services_Optional_TrivialFileDelivery_service)

simple_plugin(MyServiceUser "module" NOINSTALL)

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

simple_plugin(PSTest "service" NOINSTALL)
simple_plugin(PSTestAnalyzer "module" NOINSTALL
  test_Framework_Services_Optional_PSTest_service)

simple_plugin(PSTestInterfaceImpl "service" NOINSTALL)

cet_test(PSTest_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c PSTest_t.fcl
  DATAFILES fcl/PSTest_t.fcl
  )
