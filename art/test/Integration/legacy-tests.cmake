########################################################################
# Old tests converted from CMS FWCore/Framework/test

set(test_list
  BitsPass_t
  BitsFail_t
  BitsCount_t
  FilterIgnore_t
  TrigBits0_t
  TrigBits1_t
  TrigBits2_t
  TrigBits3_t
  TrigBits4_t
)

foreach(test ${test_list})
  cet_test(${test} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ${test}.fcl
    DATAFILES
    fcl/${test}.fcl
    fcl/messageDefaults.fcl
  )
endforeach()
