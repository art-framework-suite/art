simple_plugin(EventProcessorTestSource "source")
simple_plugin(EventProcessorTestOutput "module")

# The tests here are intended to demonstrate that the correct
# EventProcessor functions are being called during the event loop (see
# the README file for an explanation of what is intended for each
# test).  In order to achieve this, the PROC_DEBUG environment
# variable is set to a value of 1, which triggers the FDEBUG(1)
# statements, comprised of the called EventProcessor function, to
# stream to STDERR.
#
# The relevant comparisons are thus between STDERR and the reference
# specified reference file in the refs/ subdirectory.  To achieve
# this, we suppress all messagefacility messages to STDOUT (see the
# fcl/message.fcl) configuration.  There are two comparisons that
# are done:
#
#   STDOUT comparison: A successful STDOUT comparison is achieved if
#   the only STDOUT statement is:
#     ^Art has completed and will exit with status 0.$
#
#   STDERR comparison: A successful STDERR comparison is achieved if
#   the filtered .err file, produced from the test, compares equal
#   with the specified reference.
#
# The specified filter, for parsing the output, is the
# 'pass_through_filter', which simply takes the input and filters
# none of it.  The reason the default filter is not used is because
# some of the FDEBUG printout includes repeated '.' characters that
# are then replaced by the filter, which we do not want.
#
# Debugging tests: If a test fails, please keep in mind that the
# messagefacility parameters can be adjusted so that the messages are
# not suppressed (can be achieved by omitting the #include
# "message.fcl" line in the FHiCL).  Also, bare 'std::cout' statements
# will not be suppressed.

set(base_inputs_00 a)
set(base_inputs_01 a)
set(base_inputs_02 a b c d e f g h i j k l)
set(base_inputs_03 a b c)
set(base_inputs_04 a b)
set(base_inputs_05 a b)
set(base_inputs_06 a b)
set(base_inputs_07 a b)
set(base_inputs_08 a b)
set(base_inputs_09 a b c)
set(base_inputs_10 a b c)
set(base_inputs_11 a b c)
set(base_inputs_12 a b c)
set(base_inputs_13 a b)

foreach(TESTNUMBER 00 01 02 03 04 05 06 07 08 09 10 11 12 13)

  # For each of the 'base_inputs' lists, we create two other lists;
  # one that appends ".txt" to the end of each letter--this is what
  # serves as input to the program; and the other that *additionally*
  # prepends the input source path to the letter--this is what is
  # necessary for specify the required datafiles per test.
  string(REGEX REPLACE "([^;]+)" "\\1.txt" inputs_${TESTNUMBER} "${base_inputs_${TESTNUMBER}}")
  string(REGEX REPLACE "([^;]+)" "inputs/${TESTNUMBER}/\\1.txt" prepended_inputs_${TESTNUMBER} "${base_inputs_${TESTNUMBER}}")

  # For each test, loop over the cases where the behavior to
  # handleEmptyRuns and handleEmptySubRuns is [true, true], [true,false],
  # [false, true], and [false, false].
  foreach(HANDLEEMPTY tt tf ft ff)

    # Create specific FHiCL files from the ".fcl.in" files.  We do
    # this by stripping '.in' and insert the boolean values
    # (e.g. 'tf').
    set(ifclfile "${CMAKE_CURRENT_SOURCE_DIR}/fcl/eventLoop_${TESTNUMBER}.fcl.in")
    string(REGEX REPLACE ".*/([^/]+)\\.fcl\\.in$" "\\1_${HANDLEEMPTY}.fcl" fclfile ${ifclfile})
    configure_file(${ifclfile} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNUMBER}/${fclfile} @ONLY)

    cet_test(EventLoop_${TESTNUMBER}_${HANDLEEMPTY} HANDBUILT
      TEST_EXEC art
      TEST_ARGS --config eventLoop_${TESTNUMBER}_${HANDLEEMPTY}.fcl -s ${inputs_${TESTNUMBER}}
      DATAFILES
      fcl/message.fcl
      fcl/handleEmptyRunsAndSubRuns_${HANDLEEMPTY}.fcl
      ${CMAKE_CURRENT_BINARY_DIR}/${TESTNUMBER}/eventLoop_${TESTNUMBER}_${HANDLEEMPTY}.fcl
      ${prepended_inputs_${TESTNUMBER}}
      REF "${CMAKE_CURRENT_SOURCE_DIR}/EventLoop_t.out" "${CMAKE_CURRENT_SOURCE_DIR}/refs/${TESTNUMBER}/EventLoop_${HANDLEEMPTY}-ref.txt"
      OUTPUT_FILTERS "${CMAKE_CURRENT_SOURCE_DIR}/pass_through_filter"
      TEST_PROPERTIES ENVIRONMENT PROC_DEBUG=1
      )
  endforeach()
endforeach()
