cet_test(FloatingPointControl_t
  DATAFILES FloatingPointControl_t.fcl
  TEST_ARGS FloatingPointControl_t.fcl
  LIBRARIES PRIVATE
    Catch2::Catch2
    messagefacility::MF_MessageLogger
    messagefacility::MF_catch_main
    messagefacility::messagefacility_plugins_stringstream_mfPlugin
    art::art_Framework_Services_System_FloatingPointControl_service
    art::art_Framework_Services_Registry
  )
