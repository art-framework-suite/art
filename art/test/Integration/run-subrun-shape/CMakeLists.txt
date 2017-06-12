#================================================================
#
# Product-shape testing (run and subrun products)
#
#================================================================
#
# The purpose of these tests is to determine if the product shapes in
# two input files are consistent.  Consistency in product shapes is
# what determines if two input files can be concatenated together in
# the same input process.  The consistency criteria are as follows:
#
# 3. For art processes that write output files, each input file must
#    contain the SAME set of products.  Namely, a second input file
#    with fewer products in it will result in an exception thrown.
#
# In what follows, "Extra" means that extra products are produced/put
# in the same process that reads the input files.

simple_plugin(InputProducerNoEvents "module" NO_INSTALL)

#================================================================
# (1)      a.root  b.root  ----> PASS
#          ======  ======
# Runs   : p1: r1  p1: r3
#          p2: r2  p2: r4
# SubRuns: p1: s1  p1: s3
#          p2: s2  p2: s4
# Events : ------  ------
#
# Extra : NO
# Output: NO

cet_test(DifferentRunSubrunShapes_w HANDBUILT
  TEST_EXEC ./make_input_different_shapes.sh
  DATAFILES
  make_input_producer_noevents.fcl
  make_input_different_shapes.sh)

cet_test(DifferentRunSubrunShapes_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../DifferentRunSubrunShapes_w.d/a.root
  -s ../DifferentRunSubrunShapes_w.d/b.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS DifferentRunSubrunShapes_w)

# (2)      a.root  b.root  ----> PASS
#          ======  ======
# Runs   : p1: r1  p1: r1
#          p2: r2  p2: r2
# SubRuns: p1: s1  p1: s1
#          p2: s2  p2: s2
# Events : ------  ------
#
# Extra : NO
# Output: YES

cet_test(SameRunSubrunShapes_w HANDBUILT
  TEST_EXEC ./make_input_same_shapes.sh
  DATAFILES
  make_input_producer_noevents.fcl
  make_input_same_shapes.sh)

cet_test(SameRunSubrunShapes_r HANDBUILT
  # This test will produce an output file with duplicate events.
  # Since we are just comparing shapes in this test, that is okay.
  # But reading the output file from this test will result in an
  # exception throw.
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SameRunSubrunShapes_w.d/a.root
  -s ../SameRunSubrunShapes_w.d/b.root
  -o out.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS SameRunSubrunShapes_w)

# (3)      a.root  b.root  ----> PASS
#          ======  ======
# Runs   : p1: r1  p1: r2
#          p2: r2  p2: r1
# SubRuns: p1: s1  p1: s2
#          p2: s2  p2: s1
# Events : ------  ------
#
# Extra : NO
# Output: YES

cet_test(OppositeRunSubrunShapes_w HANDBUILT
  TEST_EXEC ./make_input_opposite_shapes.sh
  DATAFILES
  make_input_producer_noevents.fcl
  make_input_opposite_shapes.sh)

cet_test(OppositeRunSubrunShapes_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../OppositeRunSubrunShapes_w.d/a.root
  -s ../OppositeRunSubrunShapes_w.d/b.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS OppositeRunSubrunShapes_w)

# (4)      a.root  b.root
#          ======  ======
# Run    : p1: r1  p1: r1
#          p2: r2
# SubRuns: p1: s1  p1: s1
#          p2: s2
# Events : ------  ------

cet_test(SubsetRunSubrunShapes_w HANDBUILT
  TEST_EXEC ./make_input_subset_shapes.sh
  DATAFILES
  make_input_producer_noevents.fcl
  make_input_subset_shapes.sh)

# (4-00) ----> FAIL
# Extra : NO
# Output: YES

cet_test(SubsetRunSubrunShapes_00_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunShapes_w.d/a.root
  -s ../SubsetRunSubrunShapes_w.d/b.root
  -o out.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunShapes_w
  PASS_REGULAR_EXPRESSION "FatalRootError")

# (4-01) ----> PASS
# Extra : NO
# Output: NO

cet_test(SubsetRunSubrunShapes_01_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunShapes_w.d/a.root
  -s ../SubsetRunSubrunShapes_w.d/b.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunShapes_w)

# (4-02) ----> PASS
# Input file order: 'b.root' -> 'a.root'
# Extra : NO
# Output: NO

cet_test(SubsetRunSubrunShapes_02_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunShapes_w.d/b.root
  -s ../SubsetRunSubrunShapes_w.d/a.root
  -n 20
  DATAFILES
  empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunShapes_w)
