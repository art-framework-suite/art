cet_test(FloatingPointControl_t
  DATAFILES FloatingPointControl_t.fcl
  TEST_ARGS FloatingPointControl_t.fcl
  LIBRARIES
  mf_catch_main
  messagefacility_plugins_stringstream_mfPlugin
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_Registry
  fhiclcpp)

target_include_directories(FloatingPointControl_t PUBLIC $ENV{CATCH_INC})
