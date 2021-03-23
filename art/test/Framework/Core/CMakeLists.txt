set(default_test_libraries
  art::Framework_Services_Registry
  art::Framework_Core
  art::Framework_Principal
  art::Utilities
  ${CPPUNIT}
  ${CMAKE_DL_LIBS}
 )

set(pmt_libraries
  art::Framework_Principal
  canvas::canvas
  fhiclcpp::fhiclcpp
)
foreach (type IN ITEMS Analyzer Filter Output Producer)
  build_plugin(PMTest${type} "module" NO_INSTALL BASENAME_ONLY
    LIBRARIES PRIVATE ${pmt_libraries}
)
endforeach()
target_link_libraries(PMTestOutput_module PRIVATE fhiclcpp::types)

build_plugin(CheckTriggerBits "module" NO_INSTALL USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art::Framework_Services_System_TriggerNamesService_service
    art::Framework_Principal
    canvas::canvas
)

cet_test(PathManager_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art::Framework_Art
    art::Framework_Core
    art::Framework_Principal
    art::Framework_Services_Registry
    canvas::canvas
    fhiclcpp::fhiclcpp
)

set(eventSelector_test_libraries
  art::Framework_Services_System_TriggerNamesService_service
  art::Framework_Core
  art::Framework_Services_Registry
  canvas::canvas
  fhiclcpp::fhiclcpp
)
foreach(test IN ITEMS ector Wildcard Exc)
  cet_test(EventSel${test}_t
    SOURCE EventSel${test}_t.cpp
    LIBRARIES PRIVATE
    ${eventSelector_test_libraries}
  )
endforeach()

build_plugin(Busy "module" NO_INSTALL USE_BOOST_UNIT)
cet_test(BusyEvent_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c busy_event_t.fcl -j3
  DATAFILES fcl/busy_event_t.fcl)

cet_test(RegistryTemplate_t
  SOURCE RegistryTemplate_t.cpp
  LIBRARIES PRIVATE art::Framework_Services_Registry
)

cet_test(parse_path_spec_t USE_BOOST_UNIT
  LIBRARIES PRIVATE art::Framework_Core)

build_plugin(EventCounter "module" NO_INSTALL USE_BOOST_UNIT
  LIBRARIES PRIVATE art::Framework_Principal fhiclcpp::types)

cet_test(RejectEvents_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c reject_events_t.fcl
  DATAFILES fcl/reject_events_t.fcl
)

cet_test(GroupSelector_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art::Framework_Core
    art::Persistency_Provenance
    art::Version
    canvas::canvas
    fhiclcpp::fhiclcpp
)

cet_test(ProductRegistryHelper_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art::Framework_Core
    canvas::canvas
)

cet_test(trigger_bits_01 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_01.fcl
  DATAFILES trigger_bits_01.fcl)

cet_test(trigger_bits_02 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_02.fcl
  DATAFILES trigger_bits_02.fcl)

add_subdirectory(data-dependencies)
