set(CCV_DEFAULT_RECURSIVE FALSE)

simple_plugin(BadAssnsProducer "module" NO_INSTALL)

art_dictionary()

cet_test(BadAssns_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c BadAssns_t.fcl -o out.root -n 4
  DATAFILES fcl/BadAssns_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "No dictionary found for the following classes:\n.*art::Wrapper<art::Assns<arttest::DummyProduct,arttest::StringProduct,void> >\n.*Most likely they were never"
  )
