simple_plugin(PtrVectorAnalyzer           "module"  NO_INSTALL )
simple_plugin(PtrVectorProducer           "module"  NO_INSTALL )

cet_test(PtrVector_01a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config ptrvector_01a.fcl --rethrow-all
  DATAFILES
  fcl/ptrvector_01a.fcl
)

cet_test(PtrVector_01b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config ptrvector_01b.fcl --rethrow-all
  DATAFILES
  fcl/ptrvector_01b.fcl
  TEST_PROPERTIES DEPENDS PtrVector_01a_t
)

cet_test(PtrVector_01c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config ptrvector_01c.fcl --rethrow-all
  DATAFILES
  fcl/ptrvector_01c.fcl
)

cet_test(PtrVector_01d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config ptrvector_01d.fcl --rethrow-all
  DATAFILES
  fcl/ptrvector_01d.fcl
  TEST_PROPERTIES DEPENDS PtrVector_01c_t
)

cet_test(PtrVector_01e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config ptrvector_01e.fcl --rethrow-all
  DATAFILES
  fcl/ptrvector_01e.fcl
  TEST_PROPERTIES DEPENDS PtrVector_01d_t
)
