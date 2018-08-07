# For the tests below, NO PRODUCTS ARE CREATED.  However, output files
# have been created, thus writing a Run/SubRun auxiliary object to the
# tree.  We can therefore test the range set for each file.
#
#==================================================================
# Process 1: p_1.root [1,5)      FileIndex: [2-4]
#            p_2.root [5,11)     FileIndex: [6,9-10]
#            p_3.root [11,16)    FileIndex: [12,14-15]
#            p_4.root [16,19)    FileIndex: [16-18]
#            p_5.root [19,21)    FileIndex: []
#
# Process 2a: q_1.root [1,11)     FileIndex: [2-4,6,9-10]
#             q_2.root [11,21)    FileIndex: [12,14-18]
#
#      OR
#
# Process 2b: q_1.root [1,6), [11,16) & [19,21) FileIndex: [2-4,12,14-15]
#             q_2.root [6,11) & [16,19)         FileIndex: [6,9-10,16-18]
#
#      OR
#
# Process 2c: q_1.root [19,21), [11,16) & [1,6) FileIndex: [2-4,12,14-15]
#             q_2.root [6,11) & [16,19)         FileIndex: [6,9-10,16-18]  # Don't create this one
#
# Process 3a: r_1.root [1,21)     FileIndex: [2-4,6,10,12,14-18]
#
# Process 3b: The purpose of this test is to verify that enabling
#             compact range sets when reading the output of 2b results
#             in an output file with the range sets:
#
#               r.root [1,21) & [6,19)  FileIndex: [2-4,6,10,12,14-18]
#
#             Reading "r.root" will then yield an error.

cet_test(RangeSetAggregation_p1_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c rangeSetAggregation_p1_w.fcl
  DATAFILES fcl/rangeSetAggregation_p1_w.fcl)

# Check range sets/file indexes from first process
cet_test(RangeSetAggregation_p1_r0 HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS
  --range-of-validity
  --file-index
  -s "../RangeSetAggregation_p1_w.d/out.root"
  REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p1_r0-ref.txt"
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

foreach(i RANGE 1 5)
  cet_test(RangeSetAggregation_p1_r${i} HANDBUILT
    TEST_EXEC file_info_dumper
    TEST_ARGS
    --range-of-validity
    --file-index
    -s "../RangeSetAggregation_p1_w.d/p_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p1_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p1_w
    )
endforeach()

# Process 2a
cet_test(RangeSetAggregation_p2a_w1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_1.root"
  -s "../RangeSetAggregation_p1_w.d/p_2.root"
  -o "q_1.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

cet_test(RangeSetAggregation_p2a_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_3.root"
  -s "../RangeSetAggregation_p1_w.d/p_4.root"
  -s "../RangeSetAggregation_p1_w.d/p_5.root"
  -o "q_2.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

# Check range sets/file indexes from process 2a
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p2a_r${i} HANDBUILT
    TEST_EXEC file_info_dumper
    TEST_ARGS
    --range-of-validity
    --file-index
    -s "../RangeSetAggregation_p2a_w${i}.d/q_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p2a_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p2a_w${i}
    )
endforeach()

# Process 2b
cet_test(RangeSetAggregation_p2b_w1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_1.root"
  -s "../RangeSetAggregation_p1_w.d/p_3.root"
  -s "../RangeSetAggregation_p1_w.d/p_5.root"
  -o "q_1.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

cet_test(RangeSetAggregation_p2b_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_2.root"
  -s "../RangeSetAggregation_p1_w.d/p_4.root"
  -o "q_2.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

# Check range sets/file indexes from process 2b
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p2b_r${i} HANDBUILT
    TEST_EXEC file_info_dumper
    TEST_ARGS
    --range-of-validity
    --file-index
    -s "../RangeSetAggregation_p2b_w${i}.d/q_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p2b_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p2b_w${i}
    )
endforeach()

# Process 2c
# -- The purpose of this process is to verify that the range sets
#    produced by concatenating files p_{1,3,5}.root are the same as
#    those produced by concatenating the files in reverse order.

cet_test(RangeSetAggregation_p2c_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_5.root"
  -s "../RangeSetAggregation_p1_w.d/p_3.root"
  -s "../RangeSetAggregation_p1_w.d/p_1.root"
  -o "q.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w)

cet_test(RangeSetAggregation_p2c_r HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS
  --range-of-validity
  --file-index
  -s "../RangeSetAggregation_p2c_w.d/q.root"
  # Should give the same RangeSet results as in test 'p2b_r1'
  REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p2c_r-ref.txt"
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p2c_w)

# Process 3
cet_test(RangeSetAggregation_p3a_w1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p2a_w1.d/q_1.root"
  -s "../RangeSetAggregation_p2a_w2.d/q_2.root"
  -o "r_1.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES)

cet_test(RangeSetAggregation_p3a_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p2b_w1.d/q_1.root"
  -s "../RangeSetAggregation_p2b_w2.d/q_2.root"
  -o "r_2.root"
  DATAFILES fcl/empty.fcl
  TEST_PROPERTIES)

SET_TESTS_PROPERTIES(RangeSetAggregation_p3a_w1 PROPERTIES
  DEPENDS "RangeSetAggregation_p2a_w1;RangeSetAggregation_p2a_w2"
)

SET_TESTS_PROPERTIES(RangeSetAggregation_p3a_w2 PROPERTIES
  DEPENDS "RangeSetAggregation_p2b_w1;RangeSetAggregation_p2b_w2"
)

# Check range sets/file indexes from process 3
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p3a_r${i} HANDBUILT
    TEST_EXEC file_info_dumper
    TEST_ARGS
    --range-of-validity
    --file-index
    -s "../RangeSetAggregation_p3a_w${i}.d/r_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p3a_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p3a_w${i})
endforeach()

cet_test(RangeSetAggregation_p3a_w3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c rangeSetAggregation_p3a_w3.fcl
  -s "../RangeSetAggregation_p2b_w1.d/q_1.root"
  DATAFILES fcl/rangeSetAggregation_p3a_w3.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p2b_w1)

cet_test(RangeSetAggregation_p3b_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c rangeSetAggregation_p3b_w.fcl
  -o "r.root"
  -s "../RangeSetAggregation_p2b_w1.d/q_1.root"
  -s "../RangeSetAggregation_p2b_w2.d/q_2.root"
  DATAFILES fcl/rangeSetAggregation_p3b_w.fcl)

SET_TESTS_PROPERTIES(RangeSetAggregation_p3b_w PROPERTIES
  DEPENDS "RangeSetAggregation_p2a_w1;RangeSetAggregation_p2a_w2")

# ====================================================================
# The next two tests demonstrate that an attempt to read the
# 'compactified' range sets from 'r.root' will result in an exception
# throw since the on-disk range sets overlap.  The *_r2* test below
# shows that trying to read overlapping range sets due to
# compactification cannot be circumvented just by attempting to read
# them in a 'compact' form.

cet_test(RangeSetAggregation_p3b_r1 HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS
  --range-of-validity -s
  "../RangeSetAggregation_p3b_w.d/r.root"
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p3b_w
  PASS_REGULAR_EXPRESSION "Attempt to merge event ranges that both contain one or more of the same events")

cet_test(RangeSetAggregation_p3b_r2 HANDBUILT
  TEST_EXEC file_info_dumper
  TEST_ARGS
  --range-of-validity=compact
  "../RangeSetAggregation_p3b_w.d/r.root"
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p3b_w
  PASS_REGULAR_EXPRESSION "Attempt to merge event ranges that both contain one or more of the same events")
