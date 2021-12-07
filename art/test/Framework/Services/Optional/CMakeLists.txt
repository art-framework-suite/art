cet_build_plugin(MyService art::service NO_INSTALL BASENAME_ONLY
  LIBRARIES PRIVATE messagefacility::MF_MessageLogger)

cet_build_plugin(MySharedService art::service NO_INSTALL BASENAME_ONLY)

cet_build_plugin(MyServiceUsingIface art::service NO_INSTALL BASENAME_ONLY
  LIBRARIES PRIVATE art_test::MyService_service)

cet_build_plugin(MyServiceUser art::module NO_INSTALL BASENAME_ONLY
  LIBRARIES PRIVATE art_test::MyService_service)

cet_build_plugin(ReplicatedRNG art::module NO_INSTALL BASENAME_ONLY)

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
