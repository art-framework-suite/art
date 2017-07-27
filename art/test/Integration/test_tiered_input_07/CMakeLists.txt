# The purpose of these tests is to ensure a successful workflow where
# the secondary input files were produced with a different process
# history than the primary input file.  Like test_tiered_input_06, the
# primary and secondary input files have different first process
# names:
#
#    Primary file: ProcA -> ProcC
#    Second. file: ProcB
#
# For this test, we have the following secondary input file
# dependencies:
#
#    fC.root --> fA.root --> fB.root
#
# We attempt a lookup in fC.root for a product that is resident in
# fB.root.

cet_test(test_tiered_input_07a_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_fA.fcl --rethrow-all
  DATAFILES
  fcl/make_fA.fcl
  )

cet_test(test_tiered_input_07b_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_fB.fcl --rethrow-all
  DATAFILES
  fcl/make_fB.fcl
  )

cet_test(test_tiered_input_07c_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config make_fC.fcl --rethrow-all
  DATAFILES
  fcl/make_fC.fcl
  TEST_PROPERTIES
  DEPENDS test_tiered_input_07b_t
  )

cet_test(test_tiered_input_07d_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config read.fcl --rethrow-all
  DATAFILES
  fcl/read.fcl
  )

set_tests_properties(test_tiered_input_07d_t
  PROPERTIES DEPENDS "test_tiered_input_07a_t;test_tiered_input_07c_t")

cet_test(test_tiered_input_07e_t HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS --process-history ../test_tiered_input_07_t.d/out.root
  REF "${CMAKE_CURRENT_SOURCE_DIR}/test_tiered_input_07-ref.txt"
  TEST_PROPERTIES
  DEPENDS test_tiered_input_07d_t
  )
