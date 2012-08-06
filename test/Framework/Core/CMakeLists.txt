art_dictionary(NOINSTALL)

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

# Tests
cet_test(IntermediateTablePostProcessor_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  DATAFILES
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/fcl/test_simple_01.fcl # Avoid duplication
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Integration/fcl/messageDefaults.fcl # Avoid duplication
)
cet_test(RootDictionaryManager_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

#########################################################################
# Old (pre-ART fork) tests.

simple_plugin(TestMod module NOINSTALL ${CPPUNIT})

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

# Compile the exec (don't run). Yes, add_executable /
# target_link_libraries is also fine but this is one command with a
# uniform interface.
cet_test(Statemachine_t NO_AUTO
  SOURCES statemachine_t.cc MockEventProcessor.cc
  LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY} ${default_test_libraries}
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
