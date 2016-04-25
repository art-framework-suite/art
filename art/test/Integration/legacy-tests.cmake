########################################################################
# Old tests converted from CMS FWCore/Framework/test

# Utility script to turn an expected abort into a pass (see test
# Unscheduled_03_t below).
cet_script(test_must_abort NO_INSTALL)

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

set(test_list ${test_list}
  Unscheduled_01_t
  Unscheduled_02_t
  Unscheduled_04_t
  Unscheduled_05_t
  Unscheduled_06_t
)

# There were two more Unscheduled tests in FW that tested out-of-order
# module specification, but that is moot if we require analyzers to be
# in end paths.
foreach(test ${test_list})
  cet_test(${test} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ${test}.fcl
    DATAFILES
    fcl/${test}.fcl
    fcl/messageDefaults.fcl
  )
endforeach()

# Extra test properties
foreach(n 4;5;6)
  SET_TESTS_PROPERTIES(Unscheduled_0${n}_t PROPERTIES
    PASS_REGULAR_EXPRESSION
    "module for event:"
  )
endforeach()

SET_TESTS_PROPERTIES(Unscheduled_02_t PROPERTIES
  WILL_FAIL true
)

# This test is a little more complicated because we expect it to
# abort, which is not covered by the WILL_FAIL test property.
cet_test(Unscheduled_03_t HANDBUILT
  TEST_EXEC test_must_abort
  TEST_ARGS art --rethrow-all -c "Unscheduled_03_t.fcl"
  DATAFILES
  fcl/Unscheduled_03_t.fcl
  fcl/messageDefaults.fcl
)

cet_test_assertion("handle\\.isValid\\(\\) == require_presence_"
  Unscheduled_03_t
  )
