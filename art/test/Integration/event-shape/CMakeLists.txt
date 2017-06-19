art_dictionary(NO_INSTALL)

#================================================================
# Product-shape testing (event products)
#================================================================
#
# FIXME: For art processes that write output files, each input file
#        must contain the SAME set of products.  Namely, a second
#        input file with fewer products in it will result in an
#        exception thrown.

cet_test(DifferentHistories_ProcessA_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c produce.fcl --process-name ProcA -n 1 -o out.root
  DATAFILES
  fcl/produce.fcl)

cet_test(DifferentHistories_ProcessB_w1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c empty.fcl --process-name EMPTY -n 1 -o out.root
  DATAFILES
  fcl/empty.fcl)

cet_test(DifferentHistories_ProcessB_w2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --
  -c produce.fcl
  --process-name ProcB
  -o out.root
  -s ../DifferentHistories_ProcessB_w1.d/out.root
  DATAFILES
  fcl/produce.fcl
  TEST_PROPERTIES DEPENDS DifferentHistories_ProcessB_w1)

cet_test(DifferentHistories_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --
  -c analyze.fcl
  -s ../DifferentHistories_ProcessA_w.d/out.root
  -s ../DifferentHistories_ProcessB_w2.d/out.root
  DATAFILES
  fcl/analyze.fcl)

# Test writing out resulting products to output file
cet_test(DifferentHistories_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --
  -c analyze.fcl
  -s ../DifferentHistories_ProcessA_w.d/out.root
  -s ../DifferentHistories_ProcessB_w2.d/out.root
  -o out.root
  DATAFILES
  fcl/analyze.fcl)

set_tests_properties(DifferentHistories_r1 PROPERTIES
  DEPENDS "DifferentHistories_ProcessA_w;DifferentHistories_ProcessB_w2")

set_tests_properties(DifferentHistories_r2 PROPERTIES
  DEPENDS "DifferentHistories_ProcessA_w;DifferentHistories_ProcessB_w2"
  PASS_REGULAR_EXPRESSION "FatalRootError")

#============================================================================
# More consistency checks, just for Event-level products, also testing
# dropping on input.
#
#  FIXME: Should be more explicit about what kinds of shapes we're
#         testing here.

simple_plugin(ESPrimaryProducer   "module" NO_INSTALL)
simple_plugin(ESSecondaryProducer "module" NO_INSTALL)

# First processes: create files with two collections.
cet_test(ES_w01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_w01.fcl -n 1 -o ES_w01.root
  DATAFILES fcl/ES_w01.fcl)

cet_test(ES_w02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_w02.fcl -n 1 -o ES_w02.root
  DATAFILES fcl/ES_w02.fcl)

# Second process: derivative products.
cet_test(ES_ws01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws01.fcl -o ES_ws01.root ../ES_w01.d/ES_w01.root
  DATAFILES fcl/ES_ws01.fcl
  TEST_PROPERTIES DEPENDS ES_w01)

cet_test(ES_ws02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws02.fcl -o ES_ws02.root ../ES_w02.d/ES_w02.root
  DATAFILES fcl/ES_ws02.fcl
  TEST_PROPERTIES DEPENDS ES_w02)

# Reading processes
cet_test(ES_r01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r01.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r01.fcl)

cet_test(ES_r02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r02.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r02.fcl)

set_tests_properties(ES_r01 PROPERTIES DEPENDS "ES_ws01;ES_ws02")
set_tests_properties(ES_r02 PROPERTIES DEPENDS "ES_ws01;ES_ws02")
