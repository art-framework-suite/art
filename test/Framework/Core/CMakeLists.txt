# project name
project(art_Framework_Core_test)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/fcl/test_simple_01.fcl # Avoid duplication
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/fcl/messageDefaults.fcl # Avoid duplication
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LibraryManager_t.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  @ONLY
  )
cet_test(LibraryManager_t.sh HANDBUILT
  TEST_EXEC ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager_t.sh
  )

#########################################################################
# Old (pre-ART fork) tests.

simple_plugin(TestMod module NOINSTALL )

# cppunit tests en masse.
file(GLOB cppunit_files *.cppunit.cc)
foreach(cppunit_source ${cppunit_files})
  get_filename_component(test_name ${cppunit_source} NAME_WE )
  cet_test(${test_name} SOURCES testRunner.cpp ${cppunit_source}
    LIBRARIES ${CPPUNIT}
    )
endforeach()

foreach(cpp_test
    Event_t
    GroupSelector_t
    EventSelector_t
    EventSelWildcard_t
    EventSelExc_t
    CurrentProcessingContext_t
    CPCSentry_t
    RegistryTemplate_t
    OutputModuleUtilities_t)
  cet_test(${cpp_test} SOURCES ${cpp_test}.cpp)
endforeach()

# Compile the exec (don't run). Yes, add_executable /
# target_link_libraries is also fine but this is one command with a
# uniform interface.
cet_test(Statemachine_t NO_AUTO
  SOURCES statemachine_t.cc MockEventProcessor.cc
  LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

# Copy the script (don't run). So much easier than writing the target yourself.
cet_test(Statemachine_t.sh PREBUILT NO_AUTO)

# Shorthand to avoid writing almost the same thing three times.
macro(statemachine_test i)
  cet_test(Statemachine_t_${i} HANDBUILT
    TEST_EXEC Statemachine_t.sh
    DEPENDENCIES Statemachine_t
    DATAFILES
    unit_test_outputs/statemachine_${i}.txt
    unit_test_outputs/statemachine_output_${i}_ref.txt
    TEST_ARGS
    "statemachine_output_${i}_ref.txt"
    ${ARGN}
    -i "statemachine_${i}.txt"
    -o "statemachine_output_${i}.txt"
    )
endmacro()

# Multiple invocations of Statemachine_t.sh. We could do a couple of
# foreach clauses but then the tests would be out of order.
statemachine_test(01)
statemachine_test(02)
statemachine_test(03 -s)
statemachine_test(04 -s)
statemachine_test(05)
statemachine_test(06)
statemachine_test(07)
statemachine_test(08)
statemachine_test(09)
statemachine_test(10)
statemachine_test(11)
statemachine_test(12 -m)
