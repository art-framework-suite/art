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
  TEST_ARGS -c MyServiceUsingIface_t.fcl
  DATAFILES fcl/MyServiceUsingIface_t.fcl
  )
