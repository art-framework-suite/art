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
simple_plugin(SimpleDerivedAnalyzer    "module")
simple_plugin(SimpleDerivedProducer    "module")
simple_plugin(Throwing                 "service")

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
cet_test(PREBUILT EventSelectorFromFile_t.sh
  DATAFILES EventSelectorFromFile_w.fcl
  EventSelectorFromFile_r1.fcl
  EventSelectorFromFile_r2.fcl
  )
cet_test(test_simple_01_t.sh PREBUILT
  DATAFILES test_simple_01.fcl test_simple_01r.fcl
  )
cet_test(test_view_01_t.sh PREBUILT
  DATAFILES test_view_01a.fcl test_view_01b.fcl test_simple_01r.fcl
  )
cet_test(test_ptrvector_01_t.sh PREBUILT
  DATAFILES test_ptrvector_01a.fcl test_ptrvector_01b.fcl
  )
cet_test(test_simplederived_01_t.sh PREBUILT
  DATAFILES test_simplederived_01a.fcl test_simplederived_01b.fcl
  )
cet_test(outputCommand_t.sh PREBUILT
  DATAFILES outputCommand_w.fcl outputCommand_r.fcl
  )
cet_test(ptr_list_t.sh PREBUILT
  DATAFILES ptr_list_01.fcl
  )
cet_test(test_failingProducer_t.sh PREBUILT
  DATAFILES test_failingProducer_w.fcl test_failingProducer_r.fcl
  )
cet_test(issue_0923_t.sh PREBUILT
  DATAFILES issue_0923a.fcl issue_0923b.fcl issue_0923_ref.txt
  )
cet_test(issue_0926_t.sh PREBUILT
  DATAFILES issue_0926a.fcl issue_0926b.fcl issue_0926c.fcl
  )
cet_test(issue_0940_t.sh PREBUILT
  DATAFILES issue_0940.fcl
)
########################################################################
