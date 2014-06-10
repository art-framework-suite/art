# Tests to expose certain instances of high memory use.

art_dictionary()

simple_plugin(HMSubRunProdProducer "module" NO_INSTALL)
simple_plugin(HMRunProdProducer "module" NO_INSTALL)

cet_test(HMSubRun_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config HMSubRun_w.fcl
  DATAFILES fcl/HMSubRun_w.fcl
  OPTIONAL_GROUPS LONG
)

cet_test(HMSubRun_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config HMSubRun_r.fcl
  DATAFILES fcl/HMSubRun_r.fcl
  OPTIONAL_GROUPS LONG
  TEST_PROPERTIES DEPENDS HMSubRun_w
)
