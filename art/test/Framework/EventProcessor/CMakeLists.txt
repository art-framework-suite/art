simple_plugin(EventProcessorTest "source")
simple_plugin(EventProcessorTestOutput "module")

# WARNING ABOUT PRINTOUTS -- and how to debug if test fails

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

foreach(TESTNUMBER 01 02 03 04 05 06 07 08 09 10 11 12 13)
  string(REGEX REPLACE "([^;]+)" "\\1_${TESTNUMBER}.txt" inputs_${TESTNUMBER} "${base_inputs_${TESTNUMBER}}")
  string(REGEX REPLACE "([^;]+)" "inputs/\\1_${TESTNUMBER}.txt" prepended_inputs_${TESTNUMBER} "${base_inputs_${TESTNUMBER}}")

  foreach(HANDLEEMPTY tt tf ft ff)

    # Strip '.in' and insert the boolean values (e.g. 'tf')
    set(ifclfile "${CMAKE_CURRENT_SOURCE_DIR}/fcl/eventLoop_${TESTNUMBER}.fcl.in")
    string(REGEX REPLACE ".*/([^/]+)\\.fcl\\.in$" "\\1_${HANDLEEMPTY}.fcl" fclfile ${ifclfile})
    configure_file(${ifclfile} ${CMAKE_CURRENT_BINARY_DIR}/${fclfile} @ONLY)

    cet_test(EventLoop_${TESTNUMBER}_${HANDLEEMPTY} HANDBUILT
      TEST_EXEC art
      TEST_ARGS --config eventLoop_${TESTNUMBER}_${HANDLEEMPTY}.fcl -s ${inputs_${TESTNUMBER}}
      DATAFILES
      fcl/message.fcl
      fcl/handleEmptyRunsAndSubRuns_${HANDLEEMPTY}.fcl
      ${CMAKE_CURRENT_BINARY_DIR}/eventLoop_${TESTNUMBER}_${HANDLEEMPTY}.fcl
      ${prepended_inputs_${TESTNUMBER}}
      REF "${CMAKE_CURRENT_SOURCE_DIR}/EventLoop_t.out" "${CMAKE_CURRENT_SOURCE_DIR}/refs/EventLoop_${TESTNUMBER}_${HANDLEEMPTY}-ref.txt"
      OUTPUT_FILTERS "${CMAKE_CURRENT_SOURCE_DIR}/pass_through_filter"
      TEST_PROPERTIES ENVIRONMENT PROC_DEBUG=1
      )
  endforeach()
endforeach()
