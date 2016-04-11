cet_test(RunProductAggregation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c runProductAggregation_w.fcl
  DATAFILES
  fcl/runProductAggregation_w.fcl
  )

cet_test(RunProductAggregation_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c runProductAggregation_r.fcl
  -s "../RunProductAggregation_w.d/out.root"
  DATAFILES
  fcl/runProductAggregation_r.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregation_w
  )

cet_test(RunProductAggregationTiered_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c runProductAggregationTiered_w.fcl
  DATAFILES
  fcl/runProductAggregationTiered_w.fcl
  )

cet_test(RunProductAggregationTiered_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c runProductAggregationTiered_w2.fcl
  DATAFILES
  fcl/runProductAggregationTiered_w2.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w
  )

cet_test(RunProductAggregationTiered_w3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c runProductAggregationTiered_w3.fcl
  DATAFILES
  fcl/runProductAggregationTiered_w3.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w2
  )

cet_test(RunProductAggregationTiered_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c runProductAggregationTiered_r.fcl
  DATAFILES
  fcl/runProductAggregationTiered_r.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w
  )

cet_test(RunProductAggregationTiered_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c runProductAggregationTiered_r2.fcl
  DATAFILES
  fcl/runProductAggregationTiered_r2.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w2
  )

cet_test(RunProductAggregationTiered_r3 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c runProductAggregationTiered_r3.fcl
  DATAFILES
  fcl/runProductAggregationTiered_r3.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregationTiered_w3
  )

cet_test(SubRunProductAggregation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c subRunProductAggregation_w.fcl
  DATAFILES
  fcl/subRunProductAggregation_w.fcl
  )

cet_test(SubRunProductAggregation_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c subRunProductAggregation_r.fcl
  -s "../SubRunProductAggregation_w.d/out.root"
  DATAFILES
  fcl/subRunProductAggregation_r.fcl
  TEST_PROPERTIES DEPENDS SubRunProductAggregation_w
  )
