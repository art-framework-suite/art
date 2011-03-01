# project name
project(art_Integration_Test)

simple_plugin(DerivedPtrVectorProducer "module")
simple_plugin(DoubleProducer           "module")
simple_plugin(DoubleTestAnalyzer       "module")
simple_plugin(FailingAnalyzer          "module")
simple_plugin(FailingProducer          "module")
simple_plugin(IntProducer              "module")
simple_plugin(IntTestAnalyzer          "module")
simple_plugin(IntVectorAnalyzer        "module")
simple_plugin(IntVectorProducer        "module")
simple_plugin(MockClusterListAnalyzer  "module")
simple_plugin(MockClusterListProducer  "module")
simple_plugin(PtrListAnalyzer          "module")
simple_plugin(PtrVectorAnalyzer        "module")
simple_plugin(PtrVectorProducer        "module")
simple_plugin(Reconfiguring            "module")
simple_plugin(SimpleDerivedAnalyzer    "module")
simple_plugin(SimpleDerivedProducer    "module")
simple_plugin(TestFilter               "module")
simple_plugin(TestBitsOutput           "module")
simple_plugin(TestOutput               "module")
simple_plugin(TestResultAnalyzer       "module")

simple_plugin(Reconfigurable           "service")
simple_plugin(Throwing                 "service")

simple_plugin(ToyRawInput              "source")

# Common library link requirements for tests here.
link_libraries(
  art_Framework_CoreArt
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${CINTEX}
  ${TREE}
  ${HIST}
  ${MATRIX}
  ${NET}
  ${MATHCORE}
  ${THREAD}
  ${RIO}
  ${CORE}
  ${CINT}
  ${REFLEX}
  ${CPPUNIT}
  -ldl
 )

# cet_test macro
include(CetTest)

# Tests
cet_test(ReflexTools_t)
cet_test(EventSelectorFromFile_t.sh PREBUILT
  DATAFILES
  fcl/EventSelectorFromFile_w.fcl
  fcl/EventSelectorFromFile_r1.fcl
  fcl/EventSelectorFromFile_r2.fcl
  fcl/messageDefaults.fcl
  )
cet_test(test_simple_01_t.sh PREBUILT
  DATAFILES
  fcl/test_simple_01.fcl
  fcl/test_simple_01r.fcl
  fcl/messageDefaults.fcl
  )
cet_test(test_view_01_t.sh PREBUILT
  DATAFILES
  fcl/test_view_01a.fcl
  fcl/test_view_01b.fcl
  fcl/test_simple_01r.fcl
  fcl/messageDefaults.fcl
  )
cet_test(test_ptrvector_01_t.sh PREBUILT
  DATAFILES
  fcl/test_ptrvector_01a.fcl
  fcl/test_ptrvector_01b.fcl
  fcl/messageDefaults.fcl
  )
cet_test(test_simplederived_01_t.sh PREBUILT
  DATAFILES
  fcl/test_simplederived_01a.fcl
  fcl/test_simplederived_01b.fcl
  fcl/messageDefaults.fcl
  )
cet_test(outputCommand_t.sh PREBUILT
  DATAFILES
  fcl/outputCommand_w.fcl
  fcl/outputCommand_r.fcl
  fcl/messageDefaults.fcl
  )
cet_test(ptr_list_t.sh PREBUILT
  DATAFILES
  fcl/ptr_list_01.fcl
  fcl/messageDefaults.fcl
  )
cet_test(test_failingProducer_t.sh PREBUILT
  DATAFILES
  fcl/test_failingProducer_w.fcl
  fcl/test_failingProducer_r.fcl
  fcl/messageDefaults.fcl
  )
cet_test(issue_0923_t.sh PREBUILT
  DATAFILES
  fcl/issue_0923a.fcl
  fcl/issue_0923b.fcl 
  fcl/messageDefaults.fcl
  issue_0923_ref.txt
  )
cet_test(issue_0926_t.sh PREBUILT
  DATAFILES
  fcl/issue_0926a.fcl
  fcl/issue_0926b.fcl
  fcl/issue_0926c.fcl
  fcl/messageDefaults.fcl
  )
cet_test(issue_0940_t.sh PREBUILT
  DATAFILES
  fcl/issue_0940.fcl
  fcl/messageDefaults.fcl
  )
cet_test(reconfig_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c test_reconfig_01.fcl
  DATAFILES
  fcl/test_reconfig_01.fcl
  fcl/messageDefaults.fcl
)

########################################################################
# Old tests converted from CMS FWCore/Framework/test

# Tests expected to pass
set(pass_list
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
foreach(test ${pass_list})
  cet_test(${test} HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c ${test}.fcl
    DATAFILES
    fcl/${test}.fcl
    )
endforeach()

# Tests expected to fail
set(fail_list
)
foreach(test ${fail_list})
  cet_test(${test} HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c ${test}.fcl
    TEST_PROPERTIES WILL_FAIL true
    DATAFILES
    fcl/${test}.fcl
    )
endforeach()
