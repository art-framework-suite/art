simple_plugin(PtrVectorAnalyzer           "module"  NO_INSTALL )
simple_plugin(PtrVectorProducer           "module"  NO_INSTALL )

cet_test(test_ptrvector_01a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_ptrvector_01a.fcl --rethrow-all
  DATAFILES
  fcl/test_ptrvector_01a.fcl
)

cet_test(test_ptrvector_01b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_ptrvector_01b.fcl --rethrow-all
  DATAFILES
  fcl/test_ptrvector_01b.fcl
  TEST_PROPERTIES DEPENDS test_ptrvector_01a_t
)

cet_test(test_ptrvector_01c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_ptrvector_01c.fcl --rethrow-all
  DATAFILES
  fcl/test_ptrvector_01c.fcl
)

cet_test(test_ptrvector_01d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_ptrvector_01d.fcl --rethrow-all
  DATAFILES
  fcl/test_ptrvector_01d.fcl
  TEST_PROPERTIES DEPENDS test_ptrvector_01c_t
)

cet_test(test_ptrvector_01e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_ptrvector_01e.fcl --rethrow-all
  DATAFILES
  fcl/test_ptrvector_01e.fcl
  TEST_PROPERTIES DEPENDS test_ptrvector_01d_t
)

