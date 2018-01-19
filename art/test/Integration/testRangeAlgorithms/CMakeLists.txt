simple_plugin(AssnsProducer1   "module" NO_INSTALL cetlib_except)
simple_plugin(AssnsProducer2   "module" NO_INSTALL cetlib_except)
simple_plugin(AssnsAnalyzerFEG "module" NO_INSTALL cetlib_except)

cet_test(test_feg HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_feg.fcl
  DATAFILES test_feg.fcl
)

cet_test(test_feg2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_feg2.fcl
  DATAFILES test_feg2.fcl
  TEST_PROPERTIES DEPENDS test_feg
)

art_dictionary(DICTIONARY_LIBRARIES canvas NO_INSTALL)
