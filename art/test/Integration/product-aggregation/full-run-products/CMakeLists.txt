# Simple check that Run products do not have an aggregation function
# called.
foreach(I 1 2)
  cet_test(FullRunAggregation_w${I} HANDBUILT
    TEST_EXEC art
    TEST_ARGS
    --config fullRunAggregation_w${I}.fcl
    -o out.root
    DATAFILES
    fcl/fullRunAggregation_w.fcl
    fcl/fullRunAggregation_w${I}.fcl)
endforeach(I 1 2)

cet_test(FullRunAggregation_w3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --config empty.fcl
  -s ../FullRunAggregation_w1.d/out.root
  -s ../FullRunAggregation_w2.d/out.root
  -o out.root
  DATAFILES
  fcl/empty.fcl)

set_tests_properties(FullRunAggregation_w3 PROPERTIES DEPENDS
  "FullRunAggregation_w1;FullRunAggregation_w2")

cet_test(FullRunAggregation_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS
  --config fullRunAggregation_r.fcl
  -s ../FullRunAggregation_w3.d/out.root
  DATAFILES
  fcl/fullRunAggregation_r.fcl
  TEST_PROPERTIES DEPENDS FullRunAggregation_w3)
