art_dictionary(NO_INSTALL)

#================================================================
#
# Product-shape testing (event products)
#
#================================================================
#
# The purpose of these tests is to determine if the product shapes in
# two input files are consistent.  Consistency in product shapes is
# what determines if two input files can be concatenated together in
# the same input process.  The consistency criteria are as follows:
#
# 1. The set of products in the first input file defines the set of
#    allowed products in ALL input files.
#
# 2. For art processes that stream to output, each input file must
#    contain the SAME set of products.  Namely, a second input file
#    with fewer products in it will result in an exception thrown.
#
# 3. In addition, each Event-level data product must have been
#    produced in the same process for each input file.  In other
#    words, when comparing the BranchIDList objects from one input
#    file to the previous, each BranchIDList entry must be identical
#    to the corresponding entry in the previous file.
#
# In what follows, "Extra" means that extra products are produced/put
# in the same process that reads the input files.

simple_plugin(InputProducer           "module" NO_INSTALL)
simple_plugin(InputProducerNoEvents   "module" NO_INSTALL)
simple_plugin(InputProducerOnlyEvents "module" NO_INSTALL)

# (4)      a.root   b.root
#          ======   ======
# Runs   : p1: r1   p1: r1
#                   p2: r2
# SubRuns: p1: s1   p1: s1
#                   p2: s2
# Events : p1: b1   p1: b1
#                   p2: b2

cet_test( SubsetRunSubrunEventShapes_w HANDBUILT
  TEST_EXEC ./make_input_subset_shapes.sh
  DATAFILES
  fcl/make_input_producer.fcl
  make_input_subset_shapes.sh
  )

# (4-00) ----> FAIL
# Extra : NO
# Output: NO

cet_test( SubsetRunSubrunEventShapes_00_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunEventShapes_w.d/a.root
  -s ../SubsetRunSubrunEventShapes_w.d/b.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunEventShapes_w
  WILL_FAIL true
  )

# (4-01) ----> FAIL
# Extra : YES
# Output: NO

cet_test( SubsetRunSubrunEventShapes_01_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c make_input_producer.fcl
  --process-name=extraProducts
  -s ../SubsetRunSubrunEventShapes_w.d/a.root
  -s ../SubsetRunSubrunEventShapes_w.d/b.root
  -n 20
  DATAFILES
  fcl/make_input_producer.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunEventShapes_w
  WILL_FAIL true
  )

# (4-02) ----> PASS
# Input file order: 'b.root' -> 'a.root'
# Extra : NO
# Output: NO

cet_test( SubsetRunSubrunEventShapes_02_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunEventShapes_w.d/b.root
  -s ../SubsetRunSubrunEventShapes_w.d/a.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunEventShapes_w
  )

# (4-03)  ----> PASS
# Input file order: 'b.root' -> 'a.root'
# Extra : YES
# Output: NO

cet_test( SubsetRunSubrunEventShapes_03_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c make_input_producer.fcl
  --process-name=extraProducts
  -s ../SubsetRunSubrunEventShapes_w.d/b.root
  -s ../SubsetRunSubrunEventShapes_w.d/a.root
  -n 20
  DATAFILES
  fcl/make_input_producer.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunEventShapes_w
  )

# (4-04)  ----> FAIL
# Input file order: 'b.root' -> 'a.root'
# Extra : NO
# Output: YES

cet_test( SubsetRunSubrunEventShapes_04_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SubsetRunSubrunEventShapes_w.d/b.root
  -s ../SubsetRunSubrunEventShapes_w.d/a.root
  -o out.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS SubsetRunSubrunEventShapes_w
  WILL_FAIL true
  )

# (5)      b.root   c.root  ----> FAIL
#          ======   ======
# Runs   : p1: r1   p1: r2
#          p2: r2   p2: r1
# SubRuns: p1: s1   p1: s2
#          p2: s2   p2: s1
# Events : p1: b1   p1: b2
#          p2: b2   p2: b1
#
# Extra : NO
# Output: NO

cet_test( DifferentRunSubrunEventShapes_05_w HANDBUILT
  TEST_EXEC ./make_input_different_shapes_05.sh
  DATAFILES
  fcl/make_input_producer.fcl
  make_input_different_shapes_05.sh
  )

cet_test( DifferentRunSubrunEventShapes_05_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../DifferentRunSubrunEventShapes_05_w.d/b.root
  -s ../DifferentRunSubrunEventShapes_05_w.d/c.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS DifferentRunSubrunEventShapes_05_w
  WILL_FAIL true
  )

# (6)      b.root   d.root  ----> FAIL
#          ======   ======
# Runs   : p1: r1   p1: r1
#          p2: r2   p2: r2
# SubRuns: p1: s1   p1: s1
#          p2: s2   p2: s2
# Events : p1: b1   p1: b1
#          p2: b2   p2: b3
#
# Extra : NO
# Output: NO

cet_test( DifferentRunSubrunEventShapes_06_w HANDBUILT
  TEST_EXEC ./make_input_different_shapes_06.sh
  DATAFILES
  fcl/make_input_producer.fcl
  fcl/make_input_producer_different_shapes_06b.fcl
  fcl/make_input_producer_different_shapes_06d.fcl
  make_input_different_shapes_06.sh
  )

cet_test( DifferentRunSubrunEventShapes_06_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../DifferentRunSubrunEventShapes_06_w.d/b.root
  -s ../DifferentRunSubrunEventShapes_06_w.d/d.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS DifferentRunSubrunEventShapes_06_w
  WILL_FAIL true
  )

# (7)      a.root   b.root
#          ======   ======
# Runs   : p1: r1   p1: r1
#          p2: r2   p2: r2
# SubRuns: p1: s1   p1: s1
#          p2: s2   p2: s2
# Events : p1: b1   p1: b1
#          p2: b2   p2: b2

cet_test( SameRunSubrunEventShapes_w HANDBUILT
  TEST_EXEC ./make_input_same_shapes.sh
  DATAFILES
  fcl/make_input_producer.fcl
  make_input_same_shapes.sh
  )

# (7-00)  ----> PASS
# Extra : NO
# Output: NO

cet_test( SameRunSubrunEventShapes_00_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  -s ../SameRunSubrunEventShapes_w.d/a.root
  -s ../SameRunSubrunEventShapes_w.d/b.root
  -n 20
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS SameRunSubrunEventShapes_w
  )

# (7-01)  ----> PASS
# Extra : YES
# Output: NO

cet_test( SameRunSubrunEventShapes_01_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c make_input_producer.fcl
  --process-name=extraProducts
  -s ../SameRunSubrunEventShapes_w.d/a.root
  -s ../SameRunSubrunEventShapes_w.d/b.root
  -n 20
  DATAFILES
  fcl/make_input_producer.fcl
  TEST_PROPERTIES
  DEPENDS SameRunSubrunEventShapes_w
  )

# (7-02)  ----> PASS
# Extra : YES
# Output: YES

cet_test( SameRunSubrunEventShapes_02_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c make_input_producer.fcl
  --process-name=extraProducts
  -s ../SameRunSubrunEventShapes_w.d/a.root
  -s ../SameRunSubrunEventShapes_w.d/b.root
  -o out.root
  -n 20
  DATAFILES
  fcl/make_input_producer.fcl
  TEST_PROPERTIES
  DEPENDS SameRunSubrunEventShapes_w
  )

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
  DATAFILES fcl/ES_w01.fcl
  )

cet_test(ES_w02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_w02.fcl -n 1 -o ES_w02.root
  DATAFILES fcl/ES_w02.fcl
  )

# Second process: derivative products.
cet_test(ES_ws01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws01.fcl -o ES_ws01.root ../ES_w01.d/ES_w01.root
  DATAFILES fcl/ES_ws01.fcl
  TEST_PROPERTIES DEPENDS ES_w01
)

cet_test(ES_ws02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws02.fcl -o ES_ws02.root ../ES_w02.d/ES_w02.root
  DATAFILES fcl/ES_ws02.fcl
  TEST_PROPERTIES DEPENDS ES_w02
)

# Reading processes
cet_test(ES_r01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r01.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r01.fcl
  )

cet_test(ES_r02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r02.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r02.fcl
  )

set_tests_properties(ES_r01 PROPERTIES
  DEPENDS "ES_ws01;ES_ws02"
  PASS_REGULAR_EXPRESSION "---- MismatchedInputFiles BEGIN\n"
)

set_tests_properties(ES_r02 PROPERTIES
  DEPENDS "ES_ws01;ES_ws02"
  PASS_REGULAR_EXPRESSION "---- MismatchedInputFiles BEGIN\n"
)
