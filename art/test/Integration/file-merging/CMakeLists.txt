simple_plugin(EventIDPrinter "module" NO_INSTALL)

cet_test(ToyProductMerger_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyProductMerger_t.fcl
  DATAFILES
  ToyProductMerger_t.fcl
  )

cet_test(ToyProductNoMerger_t HANDBUILT
  TEST_EXEC ./file_no_merge.sh
  DATAFILES
  file_no_merge.sh
  eventIDPrinter.fcl
  ToyProductNoMerger_t.fcl
  )

SET_TESTS_PROPERTIES(ToyProductMerger_t PROPERTIES
  DEPENDS "ToyProductProducer_t_01;ToyProductProducer_t_02"
  )

SET_TESTS_PROPERTIES(ToyProductNoMerger_t PROPERTIES
  DEPENDS "ToyProductProducer_t_01;ToyProductProducer_t_02"
  )
