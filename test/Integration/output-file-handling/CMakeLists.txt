# Produce outputs
cet_test(SingleOutputProducerEmptyEvent_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c singleOutputProducerEmptyEvent_w.fcl
  DATAFILES
  fcl/singleOutputProducerEmptyEvent_w.fcl
  )

cet_test(MultiOutputProducerEmptyEvent_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c multiOutputProducerEmptyEvent_w.fcl
  DATAFILES
  fcl/multiOutputProducerEmptyEvent_w.fcl
  )

cet_test(MultiOutputProducerRootInput_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c multiOutputProducerRootInput_w.fcl -s ../SingleOutputProducerEmptyEvent_w.d/out.root
  DATAFILES
  fcl/multiOutputProducerRootInput_w.fcl
  )

SET_TESTS_PROPERTIES(MultiOutputProducerRootInput_w PROPERTIES
  DEPENDS "SingleOutputProducerEmptyEvent_w"
  )

# The following tests exercise the state machine in how it handles
# output-file switches, based on the outputs produced above.  The
# expected output for Event-, SubRun- and Run-level switches should be
# the same for both the 'EmptyEvent' and 'RootInput' sources.
#
# A test for output-file switching upon 'InputFile' boundaries is
# relevant only for RootInput and is presented after these.

foreach(source EmptyEvent RootInput)

  ## =============================================================
  ## Event-level output-file rollover
  SET(OutputFilePath "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducer${source}_w.d/")

  cet_test(MultiOutput${source}_FileIndexCheck_Event_t HANDBUILT
    TEST_EXEC dump_file_index
    TEST_ARGS
    ${OutputFilePath}/out_r1_sr0_e1.root
    ${OutputFilePath}/out_r1_sr0_e2.root
    ${OutputFilePath}/out_r1_sr0_e3.root
    ${OutputFilePath}/out_r1_sr0_e4.root
    ${OutputFilePath}/out_r1_sr0_e5.root
    ${OutputFilePath}/out_r1_sr1_e6.root
    ${OutputFilePath}/out_r1_sr1_e7.root
    ${OutputFilePath}/out_r1_sr1_e8.root
    ${OutputFilePath}/out_r1_sr1_e9.root
    ${OutputFilePath}/out_r1_sr1_e10.root
    ${OutputFilePath}/out_r2_sr0_e11.root
    ${OutputFilePath}/out_r2_sr0_e12.root
    ${OutputFilePath}/out_r2_sr0_e13.root
    ${OutputFilePath}/out_r2_sr0_e14.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutput_FileIndexCheck_Event_t-ref.txt"
    )

  SET_TESTS_PROPERTIES(MultiOutput${source}_FileIndexCheck_Event_t PROPERTIES
    DEPENDS "MultiOutputProducer${source}_w"
    )

  ## =============================================================
  ## SubRun-level output-file rollover

  cet_test(MultiOutput${source}_FileIndexCheck_SubRun_t HANDBUILT
    TEST_EXEC dump_file_index
    TEST_ARGS
    ${OutputFilePath}/out_r1_sr0.root
    ${OutputFilePath}/out_r1_sr1.root
    ${OutputFilePath}/out_r2_sr0.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutput_FileIndexCheck_SubRun_t-ref.txt"
    )

  SET_TESTS_PROPERTIES(MultiOutput${source}_FileIndexCheck_SubRun_t PROPERTIES
    DEPENDS "MultiOutputProducer${source}_w"
    )

  ## =============================================================
  ## Run-level output-file rollover

  cet_test(MultiOutput${source}_FileIndexCheck_Run_t HANDBUILT
    TEST_EXEC dump_file_index
    TEST_ARGS
    ${OutputFilePath}/out_r1.root
    ${OutputFilePath}/out_r2.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutput_FileIndexCheck_Run_t-ref.txt"
    )

  SET_TESTS_PROPERTIES(MultiOutput${source}_FileIndexCheck_Run_t PROPERTIES
    DEPENDS "MultiOutputProducer${source}_w"
    )

  ## =============================================================
  ## No rollover at all

  cet_test(MultiOutput${source}_FileIndexCheck_t HANDBUILT
    TEST_EXEC dump_file_index
    TEST_ARGS ${OutputFilePath}/out.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutput_FileIndexCheck_t-ref.txt"
    )

  SET_TESTS_PROPERTIES(MultiOutput${source}_FileIndexCheck_t PROPERTIES
    DEPENDS "MultiOutputProducer${source}_w"
    )

endforeach()

## =============================================================
## Output switch on InputFile boundary

SET(InputFilePath "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducerEmptyEvent_w.d")

cet_test(MultiInput_SwitchOutputOnNewInput_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  -c multiOutputProducer_SwitchOutputOnNewInput_w.fcl
  -s ${InputFilePath}/out_r1.root
  -s ${InputFilePath}/out_r2.root
  DATAFILES
  fcl/multiOutputProducer_SwitchOutputOnNewInput_w.fcl
  )

SET_TESTS_PROPERTIES(MultiInput_SwitchOutputOnNewInput_w PROPERTIES
  DEPENDS "MultiOutputProducerEmptyEvent_w"
  )

SET(InputFilePath "${CMAKE_CURRENT_BINARY_DIR}/MultiInput_SwitchOutputOnNewInput_w.d")

cet_test(MultiOutput_FileIndexCheck_InputFile_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS
  ${InputFilePath}/out_1.root
  ${InputFilePath}/out_2.root
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutput_FileIndexCheck_InputFile_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiOutput_FileIndexCheck_InputFile_t PROPERTIES
  DEPENDS "MultiInput_SwitchOutputOnNewInput_w"
  )

## =============================================================
## Check that the FileIndex looks reasonable for an output from a job
## that concatenated all event-level files looks reasonable.

SET(InputFilePath "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducerRootInput_w.d")

cet_test(MultiInputConcatenate_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -o out.root
  -s ${InputFilePath}/out_r1_sr0_e1.root
  -s ${InputFilePath}/out_r1_sr0_e2.root
  -s ${InputFilePath}/out_r1_sr0_e3.root
  -s ${InputFilePath}/out_r1_sr0_e4.root
  -s ${InputFilePath}/out_r1_sr0_e5.root
  -s ${InputFilePath}/out_r1_sr1_e6.root
  -s ${InputFilePath}/out_r1_sr1_e7.root
  -s ${InputFilePath}/out_r1_sr1_e8.root
  -s ${InputFilePath}/out_r1_sr1_e9.root
  -s ${InputFilePath}/out_r1_sr1_e10.root
  -s ${InputFilePath}/out_r2_sr0_e11.root
  -s ${InputFilePath}/out_r2_sr0_e12.root
  -s ${InputFilePath}/out_r2_sr0_e13.root
  -s ${InputFilePath}/out_r2_sr0_e14.root
  DATAFILES
  fcl/empty.fcl
  )

SET_TESTS_PROPERTIES(MultiInputConcatenate_t PROPERTIES
  DEPENDS "MultiOutputProducerRootInput_w"
  )

cet_test(MultiInputConcatenate_FileIndexCheck_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS ../MultiInputConcatenate_t.d/out.root
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiInputConcatenate_FileIndexCheck_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiInputConcatenate_FileIndexCheck_t PROPERTIES
  DEPENDS "MultiInputConcatenate_t"
  )
