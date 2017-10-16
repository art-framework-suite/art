include ( BasicPlugin )
simple_plugin( AssnsIterProducer1           "module"  NO_INSTALL )
simple_plugin( AssnsIterProducer2           "module"  NO_INSTALL )
#simple_plugin( AssnsIterAnalyzer           "module"  NO_INSTALL )
simple_plugin( ConstAssnsIterAnalyzer           "module"  NO_INSTALL )

#cet_test(test_iter HANDBUILT
#  TEST_EXEC lar 
#  TEST_ARGS --rethrow-all --config test_iter.fcl
#  DATAFILES test_iter.fcl
#)

cet_test(test_constiter HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config test_constiter.fcl
  DATAFILES test_constiter.fcl
)
art_dictionary(DICTIONARY_LIBRARIES cetlib NO_INSTALL)
