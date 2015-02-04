# simple_plugin(RNGS_analyzer  "module" NO_INSTALL )
# simple_plugin(RNGS_producer  "module" NO_INSTALL )
# simple_plugin(TestTFileServiceAnalyzer  "module" NO_INSTALL )

simple_plugin(UnitTestClient  "module" NO_INSTALL )

cet_test(UnitTestClient HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_fpc.fcl
  DATAFILES
  fcl/test_fpc.fcl
  # fcl/messageDefaults.fcl
)

