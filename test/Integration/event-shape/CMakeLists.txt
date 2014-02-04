# Tests of event shape consistency checking.
art_dictionary()

simple_plugin(ESPrimaryProducer "module" NO_INSTALL)
simple_plugin(ESSecondaryProducer "module" NO_INSTALL)
#simple_plugin(ESAnalyzer "module" NO_INSTALL)


# First processes: create files with two collections.
cet_test(ES_w01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_w01.fcl -n 1 -o ES_w01.root
  DATAFILES fcl/ES_w01.fcl
  )

cet_test(ES_w02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_w02.fcl -n 1 -o ES_w02.root
  DATAFILES fcl/ES_w02.fcl
  )

# Second process: derivative products.
cet_test(ES_ws01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws01.fcl -o ES_ws01.root ../ES_w01.d/ES_w01.root
  DATAFILES fcl/ES_ws01.fcl
  TEST_PROPERTIES DEPENDS ES_w01
)

cet_test(ES_ws02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_ws02.fcl -o ES_ws02.root ../ES_w02.d/ES_w02.root
  DATAFILES fcl/ES_ws02.fcl
  TEST_PROPERTIES DEPENDS ES_w02
)

# Reading processes
cet_test(ES_r01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r01.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r01.fcl
  )

cet_test(ES_r02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ES_r02.fcl ../ES_ws01.d/ES_ws01.root ../ES_ws02.d/ES_ws02.root
  DATAFILES fcl/ES_r02.fcl
  )

set_tests_properties(ES_r01 PROPERTIES
  DEPENDS "ES_ws01;ES_ws02"
  PASS_REGULAR_EXPRESSION "---- UnimplementedFeature BEGIN\n"
)

set_tests_properties(ES_r02 PROPERTIES
  DEPENDS "ES_ws01;ES_ws02"
  PASS_REGULAR_EXPRESSION "---- MismatchedInputFiles BEGIN\n"
)
