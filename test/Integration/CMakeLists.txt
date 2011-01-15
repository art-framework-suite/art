# project name
project(art_Integration_Test)

simple_plugin(DoubleProducer     "module")
simple_plugin(DoubleTestAnalyzer "module")
simple_plugin(IntProducer        "module")
simple_plugin(IntTestAnalyzer    "module")
simple_plugin(IntVectorAnalyzer  "module")
simple_plugin(IntVectorProducer  "module")

# Library link requirements for tests here
link_libraries(
  art_Framework_CoreArt
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
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
)

set( TEST_LIST
 ReflexTools_t
)

set( ALSO_BUILD
)

set( BOOST_TEST_ALSO_BUILD
)

set( MANUALLY_BUILT_TESTS
)

set ( ALSO_RUN
)

set( MANUALLY_RUN_TESTS
)

set( TEST_SCRIPTS
  EventSelectorFromFile_t.sh
  test_simple_01_t.sh
  test_view_01_t.sh
  outputCommand_t.sh
)

set( INPUT_FILES
  test_simple_01.fcl
  test_simple_01r.fcl
  test_view_01.fcl
  EventSelectorFromFile_w.fcl
  EventSelectorFromFile_r1.fcl
  EventSelectorFromFile_r2.fcl
  outputCommand_w.fcl
  outputCommand_r.fcl
)

# ----------------------------------------------------------------------
# Build most tests:

foreach( test ${TEST_LIST} ${ALSO_BUILD} )
  add_executable( ${test} ${test}.cc )
endforeach( test ${TEST_LIST} ${ALSO_BUILD} )

# ----------------------------------------------------------------------
# Build BOOST tests

foreach( test ${BOOST_TEST_LIST} ${BOOST_TEST_ALSO_BUILD} )
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

# ----------------------------------------------------------------------
# Deal with TEST_SCRIPTS
foreach (script ${TEST_SCRIPTS} )
  CONFIGURE_FILE ( ${script} ${EXECUTABLE_OUTPUT_PATH} COPYONLY )
  add_test( ${script} ${EXECUTABLE_OUTPUT_PATH}/${script} )
endforeach (script ${TEST_SCRIPTS} )

# ----------------------------------------------------------------------
# Copy Configuration and other input files to the correct location
foreach (input ${INPUT_FILES} )
  CONFIGURE_FILE ( ${input} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY )
endforeach (input ${INPUT_FILES} )

# ======================================================================

#########################################################################
