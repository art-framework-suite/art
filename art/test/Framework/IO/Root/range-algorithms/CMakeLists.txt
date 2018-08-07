simple_plugin(AssnsProducer1   "module" NO_INSTALL)
simple_plugin(AssnsProducer2   "module" NO_INSTALL)
simple_plugin(AssnsAnalyzerFEG "module" NO_INSTALL)

cet_test(for_each_group_1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config for_each_group_1.fcl
  DATAFILES for_each_group_1.fcl)

cet_test(for_each_group_2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config for_each_group_2.fcl
  DATAFILES for_each_group_2.fcl
  TEST_PROPERTIES DEPENDS for_each_group_1)

art_dictionary(DICTIONARY_LIBRARIES canvas NO_INSTALL)
