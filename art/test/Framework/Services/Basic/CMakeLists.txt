art_make_library(SOURCE fpc_utils.cc)

simple_plugin(FPCTest "module" NO_INSTALL
  LIBRARIES PRIVATE
    art::art_test_Framework_Services_Basic
    messagefacility::MF_MessageLogger
    Threads::Threads)

foreach(FPEX IN ITEMS DivByZero Invalid UnderFlow OverFlow)
  cet_test(FPCTest_${FPEX} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all --config test_fpc_${FPEX}_t.fcl
    DATAFILES fcl/test_fpc_${FPEX}_t.fcl)
endforeach()
