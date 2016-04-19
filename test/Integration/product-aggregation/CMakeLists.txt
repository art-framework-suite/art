# For the tests below, PRODUCTS ARE CREATED.  However, output files
# have been created, thus writing a Run/SubRun auxiliary object to the
# tree.  We can therefore test the range set for each file.

art_dictionary(NO_INSTALL)
simple_plugin(EventGenerator    "module" NO_INSTALL)
simple_plugin(ParticleSimulator "module" NO_INSTALL)
simple_plugin(Reconstruction    "module" NO_INSTALL)
simple_plugin(CheckProducts     "module" NO_INSTALL USE_BOOST_UNIT)

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

# Process 3: r_1.root [1,21)     FileIndex: [2-4,6,10,12,14-18]


cet_test(EventGeneration_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c eventGeneration_w.fcl
  DATAFILES
  fcl/eventGeneration_w.fcl
  )

cet_test(ParticleSimulation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c particleSimulation_w.fcl
  DATAFILES
  fcl/particleSimulation_w.fcl
  TEST_PROPERTIES
  DEPENDS EventGeneration_w
  )

cet_test(Reconstruction_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c reconstruction_w.fcl
  DATAFILES
  fcl/reconstruction_w.fcl
  TEST_PROPERTIES
  DEPENDS ParticleSimulation_w
  )

cet_test(Concatenate_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c concatenate_w.fcl
  DATAFILES
  fcl/concatenate_w.fcl
  TEST_PROPERTIES
  DEPENDS Reconstruction_w
  )

cet_test(CheckProducts_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c checkProducts_r.fcl
  DATAFILES
  fcl/checkProducts_r.fcl
  TEST_PROPERTIES
  DEPENDS Concatenate_w
  )

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
