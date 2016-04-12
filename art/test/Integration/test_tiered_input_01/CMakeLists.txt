cet_test(test_tiered_input_01a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_make_m1.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_make_m1.fcl
  )

cet_test(test_tiered_input_01b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_read_m1.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_read_m1.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01a_t
  )

cet_test(test_tiered_input_01c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_make_m2.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_make_m2.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01a_t
  )

cet_test(test_tiered_input_01d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_read_m2.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_read_m2.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01c_t
  )

cet_test(test_tiered_input_01e_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_make_m3.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_make_m3.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01c_t
  )

cet_test(test_tiered_input_01f_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_read_both.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_read_both.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01e_t
  )

cet_test(test_tiered_input_01g_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_copy_both.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_copy_both.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01e_t
  )

cet_test(test_tiered_input_01h_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config test_tiered_input_01_copy_both_no_fastclone.fcl --rethrow-all
  DATAFILES
  fcl/test_tiered_input_01_copy_both_no_fastclone.fcl
  TEST_PROPERTIES DEPENDS test_tiered_input_01e_t
  )

