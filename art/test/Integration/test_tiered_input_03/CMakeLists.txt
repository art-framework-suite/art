cet_test(test_tiered_input_03a1_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f11.fcl --rethrow-all
  DATAFILES
  fcl/make_f11.fcl
  )

cet_test(test_tiered_input_03a2_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f12.fcl --rethrow-all
  DATAFILES
  fcl/make_f12.fcl
  )

cet_test(test_tiered_input_03a3_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f13.fcl --rethrow-all
  DATAFILES
  fcl/make_f13.fcl
  )

cet_test(test_tiered_input_03b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f2.fcl --rethrow-all
  DATAFILES
  fcl/make_f2.fcl
  )

set_tests_properties(test_tiered_input_03b_t
  PROPERTIES DEPENDS "test_tiered_input_03a1_t;test_tiered_input_03a2_t;test_tiered_input_03a3_t"
  )

cet_test(test_tiered_input_03c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f3.fcl --rethrow-all
  DATAFILES
  fcl/make_f3.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_03b_t
  )

cet_test(test_tiered_input_03d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f4.fcl --rethrow-all
  DATAFILES
  fcl/make_f4.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_03c_t
  )

cet_test(test_tiered_input_03e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read.fcl --rethrow-all
  DATAFILES
  fcl/read.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_03d_t
  )
