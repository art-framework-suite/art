cet_find_package(Catch2 PRIVATE REQUIRED)
cet_test(FloatingPointControl_t
  DATAFILES FloatingPointControl_t.fcl
  TEST_ARGS FloatingPointControl_t.fcl
  LIBRARIES
  Catch2::Catch2
  MF_catch_main
  messagefacility_plugins_stringstream_mfPlugin
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_Registry
  fhiclcpp)
