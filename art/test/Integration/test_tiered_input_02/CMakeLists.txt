cet_test(test_tiered_input_02a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f1.fcl --rethrow-all
  DATAFILES
  fcl/make_f1.fcl
  )

cet_test(test_tiered_input_02b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f2.fcl --rethrow-all
  DATAFILES
  fcl/make_f2.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_02a_t
  )

cet_test(test_tiered_input_02c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f3.fcl --rethrow-all
  DATAFILES
  fcl/make_f3.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_02b_t
  )

cet_test(test_tiered_input_02d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f4.fcl --rethrow-all
  DATAFILES
  fcl/make_f4.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_02c_t
  )

cet_test(test_tiered_input_02e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read.fcl --rethrow-all
  DATAFILES
  fcl/read.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_02d_t
  )
