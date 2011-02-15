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
# cet_test macro
include(CetTest)

# Tests
cet_test(IntermediateTablePostProcessor_t USE_BOOST_UNIT
  DATAFILES
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/test_simple_01.fcl # Avoid duplication
)
cet_test(RootDictionaryManager_t USE_BOOST_UNIT)
cet_test(artapp_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_CoreArt
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  DATAFILES
  empty_config.fcl
)
cet_test(LibraryManager_t USE_BOOST_UNIT NO_AUTO)
# Cmake directives to prepare for Library_Manager_t test
# The driver script could do this if we could let it know where the tar file is.
add_custom_target(Prep_LibraryManager_t
  ${CMAKE_COMMAND} -E tar zxf ${CMAKE_CURRENT_SOURCE_DIR}/LibraryManager_t.tar.gz
  DEPENDS LibraryManager_t.tar.gz
  )
cet_test(LibraryManager_t.sh PREBUILT
  TEST_ARGS ${EXECUTABLE_OUTPUT_PATH}
  DEPENDENCIES Prep_LibraryManager_t
  )

#########################################################################
# Old (pre-ART fork) tests.
cet_test(eventprincipal_t
  SOURCES testRunner.cpp eventprincipal_t.cppunit.cc
)
foreach(cpp_test
    Event_t
    GroupSelector_t
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    EventSelOverlap_t
    CurrentProcessingContext_t
    CPCSentry_t
    RegistryTemplate_t
    OutputModuleUtilities_t)
  cet_test(${cpp_test} SOURCES ${cpp_test}.cpp)
endforeach()

foreach(driver_test
    Art_t
    Unscheduled_t
    Replace_t
    TrigBit_t
    TrigMask_t
    TestXMLSafeException_t)
  cet_test(${driver_test} SOURCES TestDriver.cpp)
endforeach()

cet_test(Statemachine_t
  SOURCES statemachine_t.cc MockEventProcessor.cc
  LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
cet_test(StatemachineSH_t SOURCES TestDriver.cpp
)
