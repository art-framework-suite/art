# project name
project(FrameworkCoreTest)

# Library link requirements for tests here
link_libraries( 
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CINTEX}
  ${TREE}
  ${HIST}
  ${MATRIX}
  ${NET}
  ${MATHCORE}
  ${THREAD}
  ${RIO}
	${CORE}
  ${CINT}
	${REFLEX}
  ${CPPUNIT}
  -ldl
 )

#########################################################################
# New tests (post-ART fork).
####################################

# ----------------------------------------------------------------------
# Identify, by category, all tests:

set( BOOST_TEST_LIST
  IntermediateTablePostProcessor_t
  RootDictionaryManager_t
  artapp_t
)

set( TEST_LIST
)

set( ALSO_BUILD
)

set( BOOST_ALSO_BUILD
  LibraryManager_t
)

set( MANUALLY_BUILT_TESTS
)

set ( ALSO_RUN
)

set( MANUALLY_RUN_TESTS
  LibraryManager_t.sh
)

set( INPUT_FILES
  empty_config.fcl
  summary_only_config.fcl
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/test_simple_01.fcl # Avoid duplication
)

# ----------------------------------------------------------------------
# Build most tests:

foreach( test ${TEST_LIST} ${ALSO_BUILD} )
  add_executable( ${test} ${test}.cc )
endforeach( test ${TEST_LIST} ${ALSO_BUILD} )

# ----------------------------------------------------------------------
# Build BOOST tests

foreach( test ${BOOST_TEST_LIST} ${BOOST_ALSO_BUILD} )
  add_executable( ${test} ${test}.cc )
  # Save having to put these in all tests.
  set_target_properties ( ${test} PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_MAIN
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    )
  target_link_libraries( ${test} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
endforeach( test ${BOOST_TEST_LIST} )

# ----------------------------------------------------------------------
# Build tests with special requirements:

# add_executable(  )
# target_link_libraries( )
target_link_libraries(artapp_t art_Framework_CoreArt ${Boost_PROGRAM_OPTIONS_LIBRARY})

# ----------------------------------------------------------------------
# Install non-built tests:

foreach( script ${ALSO_RUN} ${MANUALLY_RUN_TESTS} )
  add_custom_target( ${script} ALL
                     COMMAND ${CMAKE_COMMAND} -E
                       copy ${CMAKE_CURRENT_SOURCE_DIR}/${script}
                            ${EXECUTABLE_OUTPUT_PATH}/
                      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${script}
                   )
endforeach( script ${TEST_SCRIPTS} )

# ----------------------------------------------------------------------
# Run the automated tests:

foreach( test ${TEST_LIST} ${BOOST_TEST_LIST} ${MANUALLY_BUILT_TESTS} ${ALSO_RUN} )
  add_test( ${test} ${EXECUTABLE_OUTPUT_PATH}/${test} )
endforeach( test ${TEST_LIST} )

# ----------------------------------------------------------------------
# Run the MANUALLY_RUN_TESTS:

add_test( LibraryManager_t.sh
          ${EXECUTABLE_OUTPUT_PATH}/LibraryManager_t.sh
          ${EXECUTABLE_OUTPUT_PATH}
        )

# ----------------------------------------------------------------------
# Copy Configuration and other input files to the correct location
foreach (input ${INPUT_FILES} )
  CONFIGURE_FILE ( ${input} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY )
endforeach (input ${INPUT_FILES} )

# ======================================================================

# Cmake directives to prepare for Library_Manager_t test
add_custom_target(Prep_LibraryManager_t
  ${CMAKE_COMMAND} -E tar zxf ${CMAKE_CURRENT_SOURCE_DIR}/LibraryManager_t.tar.gz
  DEPENDS LibraryManager_t.tar.gz
)
add_dependencies (LibraryManager_t.sh Prep_LibraryManager_t)
# When this works the driver script can go away.
#set_tests_properties (LibraryManager_t PROPERTIES ENVIRONMENT XXX_PATH=12345:45678)
#set_tests_properties (LibraryManager_t.sh PROPERTIES ENVIRONMENT "BOOST_TEST_SHOW_PROGRESS=yes")
#########################################################################

# Old (pre-ART fork) tests.
set( FrameworkCoreTest_sources MockEventProcessor.cc )
set( TestOutputModule_sources stubs/TestOutputModule.cc )
set( TestFilterModule_sources stubs/TestFilterModule.cc )
set( TestTriggerNames_sources stubs/TestTriggerNames.cc )
set( TestMergeResults_sources stubs/TestMergeResults.cc )
set( TestMod_sources stubs/TestMod.cc )
set( TestToyModules_sources stubs/ToyModules.cc )
set( TestSchedulerModule1_sources stubs/TestSchedulerModule1.cc )
set( TestSchedulerModule2_sources stubs/TestSchedulerModule2.cc )
set( FrameworkCoreTestESDummyDataAnalyzer_sources stubs/TestESDummyDataAnalyzer.cc )
set( FrameworkCoreTestPRegisterModules_sources stubs/TestPRegisterModule2.cc
                                                 stubs/TestPRegisterModule1.cc
						 stubs/TestPRegisterModules.cc )
set( FrameworkCoreTestBeginEndJobAnalyzer_sources stubs/TestBeginEndJobAnalyzer.cc
                                                    stubs/TestBeginEndJobAnalyzerModule.cc )
set( FrameworkCoreTestFailuresAnalyzer_sources stubs/TestFailuresAnalyzer.cc )
set( FrameworkCoreHistoryAnalyzer_sources stubs/HistoryAnalyzer.cc )


set( TestFrameworkCore_sources testRunner.cpp
                                 DummySource.cc
				 edproducer_productregistry_callback.cc
				 generichandle_t.cppunit.cc
)

set( eventprocessor_sources  eventprocessor_t.cppunit.cc
						eventprocessor2_t.cppunit.cc
						stubs/TestBeginEndJobAnalyzer.cc )

add_executable( eventprincipal  testRunner.cpp eventprincipal_t.cppunit.cc )
add_executable( Event Event_t.cpp )
add_executable( Art TestDriver.cpp )
add_executable( Unscheduled TestDriver.cpp )
add_executable( Replace TestDriver.cpp )
add_executable( TrigBit TestDriver.cpp )
add_executable( TrigMask TestDriver.cpp )
add_executable( TestXMLSafeException TestDriver.cpp )
add_executable( GroupSelector GroupSelector_t.cpp )
add_executable( EventSelector EventSelector_t.cpp )
add_executable( EventSelWildcard EventSelWildcard_t.cpp )
add_executable( EventSelExc EventSelExc_t.cpp )
add_executable( EventSelOverlap EventSelOverlap_t.cpp )
add_executable( CurrentProcessingContext CurrentProcessingContext_t.cpp )
add_executable( CPCSentry CPCSentry_t.cpp )
add_executable( RegistryTemplate RegistryTemplate_t.cpp )
add_executable( TestFrameworkOutputModuleUtilities OutputModuleUtilities_t.cpp )
add_executable( Statemachine statemachine_t.cc MockEventProcessor.cc )
target_link_libraries(Statemachine
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
add_executable( StatemachineSH TestDriver.cpp )

add_test( eventprincipal  ${EXECUTABLE_OUTPUT_PATH}/eventprincipal )
add_test( Event ${EXECUTABLE_OUTPUT_PATH}/Event )
add_test( TestFrameworkCoreArt ${EXECUTABLE_OUTPUT_PATH}/TestFrameworkCoreArt )
add_test( Unscheduled ${EXECUTABLE_OUTPUT_PATH}/Unscheduled )
add_test( Replace ${EXECUTABLE_OUTPUT_PATH}/Replace )
add_test( TrigBit ${EXECUTABLE_OUTPUT_PATH}/TrigBit )
add_test( TrigMask ${EXECUTABLE_OUTPUT_PATH}/TrigMask )
add_test( TestXMLSafeException ${EXECUTABLE_OUTPUT_PATH}/TestXMLSafeException )
add_test( GroupSelector ${EXECUTABLE_OUTPUT_PATH}/GroupSelector )
add_test( EventSelector ${EXECUTABLE_OUTPUT_PATH}/EventSelector )
add_test( EventSelWildcard ${EXECUTABLE_OUTPUT_PATH}/EventSelWildcard )
add_test( EventSelExc ${EXECUTABLE_OUTPUT_PATH}/EventSelExc )
add_test( EventSelOverlap ${EXECUTABLE_OUTPUT_PATH}/EventSelOverlap )
add_test( CurrentProcessingContext ${EXECUTABLE_OUTPUT_PATH}/CurrentProcessingContext )
add_test( CPCSentry ${EXECUTABLE_OUTPUT_PATH}/CPCSentry )
add_test( RegistryTemplate ${EXECUTABLE_OUTPUT_PATH}/RegistryTemplate )
add_test( OutputModuleUtilities ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreOutputModuleUtilities )
add_test( Statemachine ${EXECUTABLE_OUTPUT_PATH}/Statemachine )
add_test( StatemachineSH ${EXECUTABLE_OUTPUT_PATH}/StatemachineSH )
