art_dictionary(NO_INSTALL)

# Library link requirements for tests here
set(default_test_libraries
  art_Framework_Services_Registry
  art_Framework_Core
  art_Framework_Principal
  art_Utilities
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${ROOT_CINTEX}
  ${ROOT_TREE}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_RIO}
  ${ROOT_THREAD}
	${ROOT_CORE}
  ${ROOT_CINT}
	${ROOT_REFLEX}
  ${CPPUNIT}
  -ldl
 )

#########################################################################
# New tests (post-ART fork).
####################################
# cet_test macro

cet_test(RootDictionaryManager_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

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
  ${FHICLCPP}
  ${CETLIB}
  )

#########################################################################
# Old (pre-ART fork) tests.

simple_plugin(TestMod module NO_INSTALL ${CPPUNIT})

# cppunit tests en masse.
file(GLOB cppunit_files *.cppunit.cc)
foreach(cppunit_source ${cppunit_files})
  get_filename_component(test_name ${cppunit_source} NAME_WE )
  cet_test(${test_name} SOURCES testRunner.cpp ${cppunit_source}
    LIBRARIES ${default_test_libraries} ${CPPUNIT}
    )
endforeach()

foreach(cpp_test
    GroupSelector_t
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    CurrentProcessingContext_t
    CPCSentry_t
    RegistryTemplate_t
    OutputModuleUtilities_t)
  cet_test(${cpp_test} SOURCES ${cpp_test}.cpp
    LIBRARIES ${default_test_libraries}
    )
endforeach()
