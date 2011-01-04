# project name
project(FWCoreFrameworkTest)

# Library link requirements for tests here
link_libraries( art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CINTEX}
  ${HIST}
  ${MATRIX}
  ${MATHCORE}
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
  RootDictionaryManager_t
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
set( FWCoreFrameworkTest_sources MockEventProcessor.cc )
set( TestOutputModule_sources stubs/TestOutputModule.cc )
set( TestFilterModule_sources stubs/TestFilterModule.cc )
set( TestTriggerNames_sources stubs/TestTriggerNames.cc )
set( TestMergeResults_sources stubs/TestMergeResults.cc )
set( TestMod_sources stubs/TestMod.cc )
set( TestToyModules_sources stubs/ToyModules.cc )
set( TestSchedulerModule1_sources stubs/TestSchedulerModule1.cc )
set( TestSchedulerModule2_sources stubs/TestSchedulerModule2.cc )
set( FWCoreFrameworkTestESDummyDataAnalyzer_sources stubs/TestESDummyDataAnalyzer.cc )
set( FWCoreFrameworkTestPRegisterModules_sources stubs/TestPRegisterModule2.cc
                                                 stubs/TestPRegisterModule1.cc
						 stubs/TestPRegisterModules.cc )
set( FWCoreFrameworkTestBeginEndJobAnalyzer_sources stubs/TestBeginEndJobAnalyzer.cc
                                                    stubs/TestBeginEndJobAnalyzerModule.cc )
set( FWCoreFrameworkTestFailuresAnalyzer_sources stubs/TestFailuresAnalyzer.cc )
set( FWCoreFrameworkHistoryAnalyzer_sources stubs/HistoryAnalyzer.cc )


set( TestFWCoreFramework_sources testRunner.cpp
                                 DummySource.cc
				 edproducer_productregistry_callback.cc
				 generichandle_t.cppunit.cc
#				 maker2_t.cppunit.cc
#				 maker_t.cppunit.cc
#				 productregistry.cppunit.cc
)
#set( FWCoreFrameworkTestDummyService_sources stubs/DependsOnDummyService.cc
#                                             stubs/DummyService.cc
#                                             stubs/DummyServiceE0.cc )
#set( FWCoreFrameworkTestDummyServicePlugin_sources stubs/Modules.cc )
set( TestFWCoreFrameworkeventprocessor_sources  eventprocessor_t.cppunit.cc
						eventprocessor2_t.cppunit.cc
						stubs/TestBeginEndJobAnalyzer.cc )

#add_library(FWCoreFrameworkTestDummyService SHARED ${art_Framework_Services_Registry_TestDummyService_sources} )
#add_library(FWCoreFrameworkTestDummyServicePlugin SHARED ${art_Framework_Services_Registry_TestDummyServicePlugin_sources} )

#add_executable( TestFWCoreFramework ${TestFWCoreFramework_sources} )
#add_executable( TestFWCoreFrameworkeventprocessor  ${TestFWCoreFrameworkeventprocessor_sources} )
add_executable( TestFrameworkCoreartapp artapp_t.cpp)
target_link_libraries(TestFrameworkCoreartapp art_Framework_CoreArt ${Boost_PROGRAM_OPTIONS_LIBRARY} )
add_executable( TestFWCoreFrameworkeventprincipal  testRunner.cpp eventprincipal_t.cppunit.cc )
add_executable( TestFWCoreFrameworkEvent Event_t.cpp )
# add_executable( TestFWCoreFrameworkView View_t.cpp )
add_executable( TestFWCoreFrameworkCmsRun TestDriver.cpp )
add_executable( TestFWCoreFrameworkUnscheduled TestDriver.cpp )
add_executable( TestFWCoreFrameworkReplace TestDriver.cpp )
add_executable( TestFWCoreFrameworkTrigBit TestDriver.cpp )
add_executable( TestFWCoreFrameworkTrigMask TestDriver.cpp )
add_executable( TestXMLSafeException TestDriver.cpp )
add_executable( TestFWCoreFrameworkGroupSelector GroupSelector_t.cpp )
add_executable( TestFWCoreFrameworkEventSelector EventSelector_t.cpp )
add_executable( TestFWCoreFrameworkEventSelWildcard EventSelWildcard_t.cpp )
add_executable( TestFWCoreFrameworkEventSelExc EventSelExc_t.cpp )
add_executable( TestFWCoreFrameworkEventSelOverlap EventSelOverlap_t.cpp )
add_executable( TestFWCoreFrameworkCurrentProcessingContext CurrentProcessingContext_t.cpp )
add_executable( TestFWCoreFrameworkCPCSentry CPCSentry_t.cpp )
add_executable( TestFWCoreFrameworkRegistryTemplate RegistryTemplate_t.cpp )
add_executable( TestFWCoreOutputModuleUtilities OutputModuleUtilities_t.cpp )
add_executable( TestFWCoreFrameworkStatemachine statemachine_t.cc MockEventProcessor.cc )
target_link_libraries(TestFWCoreFrameworkStatemachine
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
add_executable( TestFWCoreFrameworkStatemachineSH TestDriver.cpp )

add_test( TestFrameworkCoreartapp ${EXECUTABLE_OUTPUT_PATH}/TestFrameworkCoreartapp )
#add_test( TestFWCoreFramework ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFramework )
#add_test( TestFWCoreFrameworkeventprocessor  ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkeventprocessor )
add_test( TestFWCoreFrameworkeventprincipal  ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkeventprincipal )
add_test( TestFWCoreFrameworkEvent ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEvent )
#add_test( TestFWCoreFrameworkView ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkView )
add_test( TestFWCoreFrameworkCmsRun ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCmsRun )
add_test( TestFWCoreFrameworkUnscheduled ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkUnscheduled )
add_test( TestFWCoreFrameworkReplace ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkReplace )
add_test( TestFWCoreFrameworkTrigBit ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkTrigBit )
add_test( TestFWCoreFrameworkTrigMask ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkTrigMask )
add_test( TestXMLSafeException ${EXECUTABLE_OUTPUT_PATH}/TestXMLSafeException )
add_test( TestFWCoreFrameworkGroupSelector ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkGroupSelector )
add_test( TestFWCoreFrameworkEventSelector ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelector )
add_test( TestFWCoreFrameworkEventSelWildcard ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelWildcard )
add_test( TestFWCoreFrameworkEventSelExc ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelExc )
add_test( TestFWCoreFrameworkEventSelOverlap ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelOverlap )
add_test( TestFWCoreFrameworkCurrentProcessingContext ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCurrentProcessingContext )
add_test( TestFWCoreFrameworkCPCSentry ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCPCSentry )
add_test( TestFWCoreFrameworkRegistryTemplate ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkRegistryTemplate )
add_test( TestFWCoreOutputModuleUtilities ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreOutputModuleUtilities )
add_test( TestFWCoreFrameworkStatemachine ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkStatemachine )
add_test( TestFWCoreFrameworkStatemachineSH ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkStatemachineSH )
