cet_build_plugin(IntentionalError art::module NO_INSTALL)

cet_test(skip_event_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c skip_event_t.fcl -n 10
  DATAFILES fcl/skip_event_t.fcl)
