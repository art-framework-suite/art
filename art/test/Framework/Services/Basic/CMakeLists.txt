simple_plugin(TestTFileServiceAnalyzer "module" NO_INSTALL
  art_Framework_Services_Optional_TFileService_service ${ROOT_HIST} ${ROOT_GRAF})

simple_plugin(FPCTest "module" NO_INSTALL )

cet_test(FPCTest HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_fpc.fcl
  DATAFILES
  fcl/test_fpc.fcl
  )
