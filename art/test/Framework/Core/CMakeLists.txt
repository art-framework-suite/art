art_dictionary(NO_INSTALL)

# Library link requirements for tests here
set(default_test_libraries
  art_Framework_Services_Registry
  art_Framework_Core
  art_Framework_Principal
  art_Utilities
  ${CPPUNIT}
  ${CMAKE_DL_LIBS}
 )

#########################################################################
# New tests (post-ART fork).
####################################
# cet_test macro

foreach (type Analyzer Filter Output Producer)
  SET_SOURCE_FILES_PROPERTIES(PMTest${type}_module.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    )
  simple_plugin(PMTest${type} "module" NO_INSTALL)
endforeach()

cet_test(PathManager_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_Core
  fhiclcpp
  cetlib
  )

########################################################################
# If any cppunit tests require more libraries than the default, specify
# them in a ${target}_libs variable.
########################################################################

simple_plugin(TestMod module NO_INSTALL)
cet_test(maker2_t LIBRARIES ${default_test_libraries})

foreach(cpp_test
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    CurrentProcessingContext_t
    CPCSentry_t
    RegistryTemplate_t
    parse_path_spec_t)
  cet_test(${cpp_test} SOURCES ${cpp_test}.cpp
    LIBRARIES ${default_test_libraries}
    )
endforeach()

cet_test(GroupSelector_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
)
