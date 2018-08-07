cet_test(test_tiered_input_04a1_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f11.fcl --rethrow-all
  DATAFILES
  fcl/make_f11.fcl
  )

cet_test(test_tiered_input_04a2_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f12.fcl --rethrow-all
  DATAFILES
  fcl/make_f12.fcl
  )

cet_test(test_tiered_input_04a3_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f13.fcl --rethrow-all
  DATAFILES
  fcl/make_f13.fcl
  )

cet_test(test_tiered_input_04b1_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f21.fcl --rethrow-all
  DATAFILES
  fcl/make_f21.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04a1_t
  )

cet_test(test_tiered_input_04b2_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f22.fcl --rethrow-all
  DATAFILES
  fcl/make_f22.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04a2_t
  )

cet_test(test_tiered_input_04b3_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f23.fcl --rethrow-all
  DATAFILES
  fcl/make_f23.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04a3_t
  )

cet_test(test_tiered_input_04c1_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f31.fcl --rethrow-all
  DATAFILES
  fcl/make_f31.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04b1_t
  )

cet_test(test_tiered_input_04c2_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f32.fcl --rethrow-all
  DATAFILES
  fcl/make_f32.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04b2_t
  )

cet_test(test_tiered_input_04c3_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f33.fcl --rethrow-all
  DATAFILES
  fcl/make_f33.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04b3_t
  )

cet_test(test_tiered_input_04d1_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f41.fcl --rethrow-all
  DATAFILES
  fcl/make_f41.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04c1_t
  )

cet_test(test_tiered_input_04d2_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f42.fcl --rethrow-all
  DATAFILES
  fcl/make_f42.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04c2_t
  )

cet_test(test_tiered_input_04d3_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_f43.fcl --rethrow-all
  DATAFILES
  fcl/make_f43.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04c3_t
  )

cet_test(test_tiered_input_04e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read.fcl --rethrow-all
  DATAFILES
  fcl/read.fcl
  )

cet_test(test_tiered_input_04f_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config copy.fcl --rethrow-all
  DATAFILES
  fcl/copy.fcl
  )

cet_test(test_tiered_input_04g_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config copy_nofastcloning.fcl --rethrow-all
  DATAFILES
  fcl/copy_nofastcloning.fcl
  )

set_tests_properties(test_tiered_input_04e_t test_tiered_input_04f_t test_tiered_input_04g_t
  PROPERTIES
  DEPENDS "test_tiered_input_04d1_t;test_tiered_input_04d2_t;test_tiered_input_04d3_t"
  )

cet_test(test_tiered_input_04h_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read_out1_nofastcloning.fcl --rethrow-all
  DATAFILES
  fcl/read_out1_nofastcloning.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_04g_t
  )
