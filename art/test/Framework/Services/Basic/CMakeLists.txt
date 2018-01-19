art_make_library(SOURCE fpc_utils.cc)

simple_plugin(TestTFileService "module" NO_INSTALL
  art_Framework_Services_Optional_TFileService_service ${ROOT_HIST} ${ROOT_GRAF})

simple_plugin(TestTFileServiceNoRegister "module" NO_INSTALL
  art_Framework_Services_Optional_TFileService_service)

simple_plugin(FPCTest "module" NO_INSTALL art_test_Framework_Services_Basic)

foreach(FPEX DivByZero Invalid UnderFlow OverFlow)
  cet_test(FPCTest_${FPEX} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all --config test_fpc_${FPEX}_t.fcl
    DATAFILES
    fcl/test_fpc_${FPEX}_t.fcl)
endforeach()

foreach(NUM RANGE 1 5)
  cet_test(TFileService_t_0${NUM} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all --config TFileService_t_0${NUM}.fcl
    DATAFILES
    fcl/TFileService_t_0${NUM}.fcl)
endforeach()

# Test multiple modules on a path -- use same configuration as 01,
# with the addition of adding an art/ROOT output file.
cet_test(TFileService_t_06 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config TFileService_t_01.fcl
  -o art_root_output.root
  DATAFILES
  fcl/TFileService_t_01.fcl)

foreach(NUM 1 5 6) # N.B. No 'RANGE' for these two tests
  cet_test(TFileService_r_0${NUM} HANDBUILT
    TEST_EXEC root
    TEST_ARGS -l -b -q TestTFileService_r.cxx\("TFileService_r_0${NUM}_input.txt"\)
    DATAFILES
    TestTFileService_r.cxx
    TFileService_r_0${NUM}_input.txt
    TEST_PROPERTIES DEPENDS TFileService_t_0${NUM})
endforeach()
