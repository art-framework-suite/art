simple_plugin(MyService "service" NO_INSTALL)
simple_plugin(MySharedService "service" NO_INSTALL)

simple_plugin(MyServiceUsingIface "service" NO_INSTALL
  art_Framework_Services_Optional_TrivialFileDelivery_service)

simple_plugin(MyServiceUser "module" NO_INSTALL)
simple_plugin(ReplicatedRNG "module" NO_INSTALL)

cet_test(MyService_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MyService_t.fcl
  DATAFILES fcl/MyService_t.fcl)

cet_test(MyServiceUsingIface_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MyServiceUsingIface_t.fcl
  DATAFILES fcl/MyServiceUsingIface_t.fcl)

cet_test(ReplicatedRNG_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ReplicatedRNG_t.fcl -j3
  DATAFILES fcl/ReplicatedRNG_t.fcl)

cet_test(MyLegacyServiceImpl_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MyLegacyServiceImpl_t.fcl -j3
  DATAFILES fcl/MyLegacyServiceImpl_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Please reconfigure your job to use only one schedule/thread\.")

cet_test(MySharedServiceImpl_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MySharedServiceImpl_t.fcl -j3
  DATAFILES fcl/MySharedServiceImpl_t.fcl)
