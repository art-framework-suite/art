simple_plugin(EventIDPrinter "module" NO_INSTALL)

foreach(num 01 02)
  cet_test(ToyProductProducer_t_${num} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ToyProductProducer_t_${num}.fcl
    DATAFILES
    fcl/ToyProductProducer_t_${num}.fcl
    )
endforeach()

## =============================================================

cet_test(ToyProductMerger_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyProductMerger_t.fcl
  DATAFILES
  fcl/ToyProductMerger_t.fcl
  )

SET_TESTS_PROPERTIES(ToyProductMerger_t PROPERTIES
  DEPENDS "ToyProductProducer_t_01;ToyProductProducer_t_02"
  )

## =============================================================

cet_test(ToyProductNoMerger_t HANDBUILT
  TEST_EXEC ./file_no_merge.sh
  DATAFILES
  file_no_merge.sh
  fcl/eventIDPrinter.fcl
  fcl/ToyProductNoMerger_t.fcl
  )

SET_TESTS_PROPERTIES(ToyProductNoMerger_t PROPERTIES
  DEPENDS "ToyProductProducer_t_01;ToyProductProducer_t_02"
  )

## =============================================================

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

## =============================================================

file(GLOB eventLevelFiles "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducerEmptyEvent_w.d/out_*_e*.root")
list(SORT eventLevelFiles)

cet_test(MultiOutputFileIndexCheck_Event_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS ${eventLevelFiles}
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutputFileIndexCheck_Event_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiOutputFileIndexCheck_Event_t PROPERTIES
  DEPENDS "MultiOutputProducerEmptyEvent_w"
  )

## =============================================================

file(GLOB subRunLevelFiles "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducerEmptyEvent_w.d/out_*_sr?.root")
list(SORT subRunLevelFiles)

cet_test(MultiOutputFileIndexCheck_SubRun_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS ${subRunLevelFiles}
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutputFileIndexCheck_SubRun_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiOutputFileIndexCheck_SubRun_t PROPERTIES
  DEPENDS "MultiOutputProducerEmptyEvent_w"
  )

## =============================================================

file(GLOB runLevelFiles "${CMAKE_CURRENT_BINARY_DIR}/MultiOutputProducerEmptyEvent_w.d/out_r?.root")
list(SORT runLevelFiles)

cet_test(MultiOutputFileIndexCheck_Run_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS ${runLevelFiles}
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutputFileIndexCheck_Run_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiOutputFileIndexCheck_Run_t PROPERTIES
  DEPENDS "MultiOutputProducerEmptyEvent_w"
  )

## =============================================================

cet_test(MultiOutputFileIndexCheck_t HANDBUILT
  TEST_EXEC dump_file_index
  TEST_ARGS ../MultiOutputProducerEmptyEvent_w.d/out.root
  REF "${CMAKE_CURRENT_SOURCE_DIR}/MultiOutputFileIndexCheck_t-ref.txt"
  )

SET_TESTS_PROPERTIES(MultiOutputFileIndexCheck_t PROPERTIES
  DEPENDS "MultiOutputProducerEmptyEvent_w"
  )
