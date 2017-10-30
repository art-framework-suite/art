simple_plugin(TestTFileService "module" NO_INSTALL
  art_Framework_Services_Optional_TFileService_service ${ROOT_HIST} ${ROOT_GRAF})

simple_plugin(TestTFileServiceNoRegister "module" NO_INSTALL
  art_Framework_Services_Optional_TFileService_service)

simple_plugin(FPCTest "module" NO_INSTALL)

cet_test(FPCTest HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_fpc.fcl
  DATAFILES
  fcl/test_fpc.fcl)

foreach(NUM RANGE 1 5)
  cet_test(TFileService_t_0${NUM} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all --config TFileService_t_0${NUM}.fcl
    DATAFILES
    fcl/TFileService_t_0${NUM}.fcl)
endforeach()

foreach(NUM 1 5) # N.B. No 'RANGE' for these two tests
  cet_test(TFileService_r_0${NUM} HANDBUILT
    TEST_EXEC root
    TEST_ARGS -l -b -q TestTFileService_r.cxx\("TFileService_r_0${NUM}_input.txt"\)
    DATAFILES
    TestTFileService_r.cxx
    TFileService_r_0${NUM}_input.txt
    TEST_PROPERTIES DEPENDS TFileService_t_0${NUM})
endforeach()
