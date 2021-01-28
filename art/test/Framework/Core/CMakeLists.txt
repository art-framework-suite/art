set(default_test_libraries
  art::art_Framework_Services_Registry
  art::art_Framework_Core
  art::art_Framework_Principal
  art::art_Utilities
  ${CPPUNIT}
  ${CMAKE_DL_LIBS}
 )

cet_make_library(LIBRARY_NAME set_manager_for_tests
  SOURCE set_manager_for_tests.cc
  LIBRARIES PRIVATE art::art_Framework_Services_Registry)

foreach (type IN ITEMS Analyzer Filter Output Producer)
  simple_plugin(PMTest${type} "module" NO_INSTALL)
endforeach()
simple_plugin(CheckTriggerBits "module" NO_INSTALL USE_BOOST_UNIT
              LIBRARIES PRIVATE art::art_Framework_Services_System_TriggerNamesService_service)

cet_test(PathManager_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
  art::art_Framework_Art
  art::art_Framework_Core
)

foreach(cpp_test IN ITEMS
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    RegistryTemplate_t
    parse_path_spec_t)
  cet_test(${cpp_test} SOURCE ${cpp_test}.cpp
    LIBRARIES PRIVATE ${default_test_libraries} art::set_manager_for_tests
  )
endforeach()

simple_plugin(EventCounter "module" NO_INSTALL USE_BOOST_UNIT)
cet_test(RejectEvents_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c reject_events_t.fcl
  DATAFILES fcl/reject_events_t.fcl
)

cet_test(GroupSelector_t USE_BOOST_UNIT
  LIBRARIES PRIVATE ${default_test_libraries}
)

cet_test(ProductRegistryHelper_t USE_BOOST_UNIT
  LIBRARIES PRIVATE art::art_Framework_Core)

cet_test(trigger_bits_01 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_01.fcl
  DATAFILES trigger_bits_01.fcl)

cet_test(trigger_bits_02 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_02.fcl
  DATAFILES trigger_bits_02.fcl)

add_subdirectory(data-dependencies)
