set(default_test_libraries
  art_Framework_Services_Registry
  art_Framework_Core
  art_Framework_Principal
  art_Utilities
  ${CPPUNIT}
  ${CMAKE_DL_LIBS}
 )

cet_make_library(LIBRARY_NAME set_manager_for_tests
  SOURCE set_manager_for_tests.cc
  LIBRARIES art_Framework_Services_Registry)

foreach (type Analyzer Filter Output Producer)
  SET_SOURCE_FILES_PROPERTIES(PMTest${type}_module.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    )
  simple_plugin(PMTest${type} "module" NO_INSTALL)
endforeach()

cet_test(PathManager_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Art
  art_Framework_Core
  fhiclcpp
  cetlib
  )

foreach(cpp_test
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    RegistryTemplate_t
    parse_path_spec_t)
  cet_test(${cpp_test} SOURCES ${cpp_test}.cpp
    LIBRARIES ${default_test_libraries} set_manager_for_tests
    )
endforeach()

cet_test(GroupSelector_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
)

cet_test(ProductRegistryHelper_t USE_BOOST_UNIT
  LIBRARIES art_Framework_Core cetlib_except)

cet_test(trigger_bits_01 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_01.fcl
  DATAFILES trigger_bits_01.fcl)

cet_test(trigger_bits_02 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c trigger_bits_02.fcl
  DATAFILES trigger_bits_02.fcl)

add_subdirectory(data-dependencies)
