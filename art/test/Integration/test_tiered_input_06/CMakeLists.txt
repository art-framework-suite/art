# The purpose of these tests is to ensure a successful workflow where
# the secondary input files were produced with a different process
# history than the primary input file.  For this case, the primary and
# secondary input files have different first process names:
#
#    Primary file: ProcA
#    Second. file: ProcB
#
# This is enough to ensure that they have different histories.
# Processing the file produced from ProcA, we test that we can lookup
# products created in ProcB.

cet_test(test_tiered_input_06a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_fA.fcl --rethrow-all
  DATAFILES
  fcl/make_fA.fcl
  )

cet_test(test_tiered_input_06b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_fB.fcl --rethrow-all
  DATAFILES
  fcl/make_fB.fcl
  )

cet_test(test_tiered_input_06c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read.fcl --rethrow-all
  DATAFILES
  fcl/read.fcl
  )

cet_test(test_tiered_input_06d_t HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS --process-history ../test_tiered_input_06c_t.d/out.root
  REF "${CMAKE_CURRENT_SOURCE_DIR}/test_tiered_input_06-ref.txt"
  TEST_PROPERTIES
  DEPENDS test_tiered_input_06c_t
  )

set_tests_properties(test_tiered_input_06c_t
  PROPERTIES DEPENDS "test_tiered_input_06a_t;test_tiered_input_06b_t")
