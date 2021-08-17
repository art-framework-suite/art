cet_test(FloatingPointControl_t
  DATAFILES fcl/FloatingPointControl_t.fcl
  TEST_ARGS FloatingPointControl_t.fcl
  LIBRARIES PRIVATE
    Catch2::Catch2
    messagefacility::MF_MessageLogger
    messagefacility::MF_catch_main
    messagefacility::plugins_stringstream_mfPlugin
    art::Framework_Services_System_FloatingPointControl_service
    art::Framework_Services_Registry
  )

cet_make_library(LIBRARY_NAME art_test_fpc NO_INSTALL SOURCE fpc_utils.cc)

cet_build_plugin(FPCTest art::module NO_INSTALL
  LIBRARIES PRIVATE
    art_test::fpc
    art::Framework_Principal
    messagefacility::MF_MessageLogger
    hep_concurrency::hep_concurrency
)

foreach(FPEX IN ITEMS DivByZero Invalid UnderFlow OverFlow)
  cet_test(FPCTest_${FPEX} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all --config test_fpc_${FPEX}_t.fcl
    DATAFILES fcl/test_fpc_${FPEX}_t.fcl)
endforeach()
