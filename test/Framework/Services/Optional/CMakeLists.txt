simple_plugin(MyService "service" NOINSTALL)

simple_plugin(MyServiceUser "module" NOINSTALL)

cet_test(MyService_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c MyService_t.fcl
  DATAFILES fcl/MyService_t.fcl
  )
