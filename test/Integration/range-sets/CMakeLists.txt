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
# Process 3: r_1.root [1,21)     FileIndex: [2-4,6,10,12,14-18]


cet_test(RangeSetAggregation_p1_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c rangeSetAggregation_p1_w.fcl
  DATAFILES
  fcl/rangeSetAggregation_p1_w.fcl
  )

# Check range sets/file indexes from first process
cet_test(RangeSetAggregation_p1_r0 HANDBUILT
  TEST_EXEC dump_file_info
  TEST_ARGS
  --print-range-sets
  --print-file-index
  -s "../RangeSetAggregation_p1_w.d/out.root"
  REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p1_r0-ref.txt"
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w
  )

foreach(i RANGE 1 5)
  cet_test(RangeSetAggregation_p1_r${i} HANDBUILT
    TEST_EXEC dump_file_info
    TEST_ARGS
    --print-range-sets
    --print-file-index
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
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w
  )

cet_test(RangeSetAggregation_p2a_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_3.root"
  -s "../RangeSetAggregation_p1_w.d/p_4.root"
  -s "../RangeSetAggregation_p1_w.d/p_5.root"
  -o "q_2.root"
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w
  )

# Check range sets/file indexes from process 2a
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p2a_r${i} HANDBUILT
    TEST_EXEC dump_file_info
    TEST_ARGS
    --print-range-sets
    --print-file-index
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
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w
  )

cet_test(RangeSetAggregation_p2b_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p1_w.d/p_2.root"
  -s "../RangeSetAggregation_p1_w.d/p_4.root"
  -o "q_2.root"
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  DEPENDS RangeSetAggregation_p1_w
  )

# Check range sets/file indexes from process 2b
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p2b_r${i} HANDBUILT
    TEST_EXEC dump_file_info
    TEST_ARGS
    --print-range-sets
    --print-file-index
    -s "../RangeSetAggregation_p2b_w${i}.d/q_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p2b_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p2b_w${i}
    )
endforeach()

# Process 3
cet_test(RangeSetAggregation_p3_w1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p2a_w1.d/q_1.root"
  -s "../RangeSetAggregation_p2a_w2.d/q_2.root"
  -o "r_1.root"
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  )

cet_test(RangeSetAggregation_p3_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --rethrow-all
  -c empty.fcl
  -s "../RangeSetAggregation_p2b_w1.d/q_1.root"
  -s "../RangeSetAggregation_p2b_w2.d/q_2.root"
  -o "r_2.root"
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  )

SET_TESTS_PROPERTIES(RangeSetAggregation_p3_w1 PROPERTIES
  DEPENDS "RangeSetAggregation_p2a_w1;RangeSetAggregation_p2a_w2"
)

SET_TESTS_PROPERTIES(RangeSetAggregation_p3_w2 PROPERTIES
  DEPENDS "RangeSetAggregation_p2b_w1;RangeSetAggregation_p2b_w2"
)

# Check range sets/file indexes from process 3
foreach(i RANGE 1 2)
  cet_test(RangeSetAggregation_p3_r${i} HANDBUILT
    TEST_EXEC dump_file_info
    TEST_ARGS
    --print-range-sets
    --print-file-index
    -s "../RangeSetAggregation_p3_w${i}.d/r_${i}.root"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/ref/RangeSetAggregation_p3_r${i}-ref.txt"
    TEST_PROPERTIES
    DEPENDS RangeSetAggregation_p3_w${i}
    )
endforeach()

#cet_test(RunProductAggregation_w1 HANDBUILT
#  TEST_EXEC art
#  TEST_ARGS --rethrow-all -c runProductAggregation_w.fcl
#  DATAFILES
#  fcl/runProductAggregation_w1.fcl
#  )
#
# Tiered test 2
#==================================================================
# Process 1: p_1.root [1,5)      FileIndex: [2-4]
#            p_2.root [5,11)     FileIndex: [6,9-10]
#            p_3.root [11,16)    FileIndex: [12,14-15]
#            p_4.root [16,19)    FileIndex: [16-18]
#            p_4.root [19,21)    FileIndex: []
#
#
# Process 3: r_1.root [1,21)  FileIndex: [2-4,6,10,12,14-18]



# Check file indices from first process

#cet_test(RunProductAggregation_rangeSet_p1_r1 HANDBUILT
#  TEST_EXEC dump_range_sets
#  TEST_ARGS
#  -s "../RunProductAggregation_w.d/out.root"
#  TEST_PROPERTIES
#  DEPENDS RunProductAggregation_w
#  )
#
# cet_test(RunProductAggregation_r HANDBUILT
#   TEST_EXEC art_ut
#   TEST_ARGS
#   -- -c runProductAggregation_r.fcl
#   -s "../RunProductAggregation_w.d/out.root"
#   DATAFILES
#   fcl/runProductAggregation_r.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregation_w
#   )
#
# cet_test(RunProductAggregationTiered_w HANDBUILT
#   TEST_EXEC art
#   TEST_ARGS --rethrow-all -c runProductAggregationTiered_w.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_w.fcl
#   )
#
# cet_test(RunProductAggregationTiered_w2 HANDBUILT
#   TEST_EXEC art
#   TEST_ARGS --rethrow-all -c runProductAggregationTiered_w2.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_w2.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w
#   )
#
# cet_test(RunProductAggregationTiered_w3 HANDBUILT
#   TEST_EXEC art
#   TEST_ARGS --rethrow-all -c runProductAggregationTiered_w3.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_w3.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w2
#   )
#
# cet_test(RunProductAggregationTiered_r HANDBUILT
#   TEST_EXEC art_ut
#   TEST_ARGS
#   -- -c runProductAggregationTiered_r.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_r.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w
#   )
#
# cet_test(RunProductAggregationTiered_r2 HANDBUILT
#   TEST_EXEC art_ut
#   TEST_ARGS
#   -- -c runProductAggregationTiered_r2.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_r2.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w2
#   )
#
# cet_test(RunProductAggregationTiered_r3 HANDBUILT
#   TEST_EXEC art_ut
#   TEST_ARGS
#   -- -c runProductAggregationTiered_r3.fcl
#   DATAFILES
#   fcl/runProductAggregationTiered_r3.fcl
#   TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w3
#   )
#
# cet_test(SubRunProductAggregation_w HANDBUILT
#   TEST_EXEC art
#   TEST_ARGS --rethrow-all -c subRunProductAggregation_w.fcl
#   DATAFILES
#   fcl/subRunProductAggregation_w.fcl
#   )
#
# cet_test(SubRunProductAggregation_r HANDBUILT
#   TEST_EXEC art_ut
#   TEST_ARGS
#   -- -c subRunProductAggregation_r.fcl
#   -s "../SubRunProductAggregation_w.d/out.root"
#   DATAFILES
#   fcl/subRunProductAggregation_r.fcl
#   TEST_PROPERTIES DEPENDS SubRunProductAggregation_w
#   )
