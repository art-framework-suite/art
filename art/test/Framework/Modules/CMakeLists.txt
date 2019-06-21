cet_test(EmptyEvent_config_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config empty_event_config_t1.fcl
  DATAFILES empty_event_config_t1.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The 'maxTime' parameter cannot be used with the 'maxEvents' or 'maxSubRuns' parameters")

cet_test(EmptyEvent_config_t2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config empty_event_config_t2.fcl
  DATAFILES empty_event_config_t2.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The 'maxTime' parameter cannot be used with the 'maxEvents' or 'maxSubRuns' parameters")

cet_test(EmptyEvent_config_t3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config empty_event_config_t3.fcl
  DATAFILES empty_event_config_t3.fcl)
