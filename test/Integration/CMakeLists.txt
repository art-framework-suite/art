# project name
project(art_Integration_Test)

simple_plugin(AddIntsProducer          "module" NOINSTALL )
simple_plugin(BareStringAnalyzer       "module" NOINSTALL )
simple_plugin(BareStringProducer       "module" NOINSTALL )
simple_plugin(DerivedPtrVectorProducer "module" NOINSTALL )
simple_plugin(DoubleProducer           "module" NOINSTALL )
simple_plugin(DoubleTestAnalyzer       "module" NOINSTALL )
simple_plugin(DropOnInputTestAnalyzer  "module" NOINSTALL )
simple_plugin(FailingAnalyzer          "module" NOINSTALL )
simple_plugin(FailingProducer          "module" NOINSTALL )
simple_plugin(IntProducer              "module" NOINSTALL )
simple_plugin(IntTestAnalyzer          "module" NOINSTALL )
simple_plugin(IntVectorAnalyzer        "module" NOINSTALL )
simple_plugin(IntVectorProducer        "module" NOINSTALL )
simple_plugin(MixAnalyzer              "module" NOINSTALL )
simple_plugin(MixFilterTest            "module" art_Framework_IO_ProductMix NOINSTALL )
simple_plugin(MixProducer              "module" NOINSTALL )
simple_plugin(MockClusterListAnalyzer  "module" NOINSTALL )
simple_plugin(MockClusterListProducer  "module" NOINSTALL )
simple_plugin(ProductIDGetter          "module" NOINSTALL )
simple_plugin(ProductIDGetterAnalyzer  "module" NOINSTALL )
simple_plugin(PtrListAnalyzer          "module" NOINSTALL )
simple_plugin(PtrVectorAnalyzer        "module" NOINSTALL )
simple_plugin(PtrVectorProducer        "module" NOINSTALL )
simple_plugin(PtrmvProducer            "module" NOINSTALL )
simple_plugin(PtrmvAnalyzer            "module" ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} NOINSTALL )
simple_plugin(Reconfigurable           "service" NOINSTALL )
simple_plugin(Reconfiguring            "module" NOINSTALL )
simple_plugin(SimpleDerivedAnalyzer    "module" NOINSTALL )
simple_plugin(SimpleDerivedProducer    "module" NOINSTALL )
simple_plugin(TestBitsOutput           "module" NOINSTALL )
simple_plugin(TestFilter               "module" NOINSTALL )
simple_plugin(TestOutput               "module" NOINSTALL )
simple_plugin(TestResultAnalyzer       "module" NOINSTALL )
simple_plugin(Throwing                 "service" NOINSTALL )
simple_plugin(ToyRawInput              "source" NOINSTALL )
simple_plugin(ToyRawInputTester        "module" NOINSTALL )
simple_plugin(ToyRawProductAnalyzer    "module" NOINSTALL )
simple_plugin(U_S                      "module" ALLOW_UNDERSCORES NOINSTALL )

art_dictionary(DICTIONARY_LIBRARIES ${CETLIB} NOINSTALL)

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

cet_test(issue_0930_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_0930.fcl
  DATAFILES
  fcl/issue_0930.fcl
  fcl/messageDefaults.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "LogicError BEGIN"
)

cet_test( test_FileDumperOutput.sh PREBUILT
  DATAFILES
  fcl/FileDumperOutputTest_w.fcl
  fcl/FileDumperOutputTest_r.fcl
)

cet_test(ToyRawInput_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ToyRawInput_01.fcl
  DATAFILES
  fcl/ToyRawInput_01.fcl
  fcl/messageDefaults.fcl
 )

foreach(num 02 03 04)
  cet_test(ToyRawInput_t_${num} HANDBUILT
    TEST_EXEC test_must_abort
    TEST_ARGS art -c ToyRawInput_${num}.fcl
    DATAFILES
    fcl/ToyRawInput_${num}.fcl
    fcl/ToyRawInput_common.fcl
    fcl/messageDefaults.fcl
    )
endforeach()

SET_TESTS_PROPERTIES(ToyRawInput_t_02 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Assertion `outR \\|\\| inR' failed"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_03 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Assertion `outSR \\|\\| inSR' failed"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_04 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Assertion `outSR \\|\\| inSR' failed"
  )

foreach(num 05 06 07 08 09 10)
  cet_test(ToyRawInput_t_${num} HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c ToyRawInput_${num}.fcl
    DATAFILES
    fcl/ToyRawInput_${num}.fcl
    fcl/ToyRawInput_common.fcl
    fcl/messageDefaults.fcl
    )
endforeach()

SET_TESTS_PROPERTIES(ToyRawInput_t_05 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned a 'new' SubRun that was the same as the previous SubRun"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_06 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned a 'new' Run that was the same as the previous Run"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_07 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned a new Run and Event without a SubRun"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_08 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned true but created no new data"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_09 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned false but created new data"
  )

SET_TESTS_PROPERTIES(ToyRawInput_t_10 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "readNext returned a new Run with a SubRun from the wrong Run"
  )

cet_test(ToyRawInput_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ToyRawInput_w.fcl
  DATAFILES
  fcl/ToyRawInput_w.fcl
  fcl/ToyRawInput_wr_f1.fcl
  fcl/ToyRawInput_common.fcl
)

cet_test(ToyRawInput_r1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ToyRawInput_r1.fcl
  DATAFILES
  fcl/ToyRawInput_r1.fcl
  fcl/ToyRawInput_wr_f1.fcl
)

cet_test(ToyRawInput_r2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ToyRawInput_r2.fcl
  DATAFILES
  fcl/ToyRawInput_r2.fcl
  fcl/ToyRawInput_r1.fcl
  fcl/ToyRawInput_wr_f1.fcl
)

cet_test(ToyRawInput_r3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c ToyRawInput_r3.fcl
  DATAFILES
  fcl/ToyRawInput_r3.fcl
  fcl/ToyRawInput_r1.fcl
  fcl/ToyRawInput_wr_f1.fcl
)

cet_test(BareString_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BareString_w.fcl
  DATAFILES
  fcl/BareString_w.fcl
)

cet_test(BareString_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c BareString_r.fcl
  DATAFILES
  fcl/BareString_r.fcl
)


cet_test(ProductMix_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "ProductMix_w.fcl"
  DATAFILES
  fcl/ProductMix_w.fcl
)

cet_test(ProductMix_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "ProductMix_r1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
)

cet_test(ProductMix_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "ProductMix_r2.fcl"
  DATAFILES
  fcl/ProductMix_r2.fcl
)

cet_test(Ptrmv_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "Ptrmv_w.fcl"
  DATAFILES
  fcl/Ptrmv_w.fcl
)

cet_test(Ptrmv_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "Ptrmv_r.fcl"
  DATAFILES
  fcl/Ptrmv_r.fcl
)

if ((${CMAKE_VERSION} VERSION_EQUAL "2.8") OR
    (${CMAKE_VERSION} VERSION_GREATER "2.8"))
  # For parallel testing, enforce an order.
  SET_TESTS_PROPERTIES(BareString_r PROPERTIES DEPENDS BareString_w)
  SET_TESTS_PROPERTIES(ToyRawInput_r1 PROPERTIES DEPENDS ToyRawInput_w)
  SET_TESTS_PROPERTIES(ToyRawInput_r2 PROPERTIES DEPENDS ToyRawInput_w)
  SET_TESTS_PROPERTIES(ToyRawInput_r3 PROPERTIES DEPENDS ToyRawInput_w)
  SET_TESTS_PROPERTIES(ProductMix_r1 PROPERTIES DEPENDS ProductMix_w)
  SET_TESTS_PROPERTIES(ProductMix_r2 PROPERTIES DEPENDS ProductMix_r1)
  SET_TESTS_PROPERTIES(Ptrmv_r PROPERTIES DEPENDS Ptrmv_w)
endif()

cet_test(ProductIDGetter_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "ProductIDGetter_t.fcl"
  DATAFILES
  fcl/ProductIDGetter_t.fcl
)

cet_test(DropOnInput_null_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "DropOnInput_null_t.fcl"
  DATAFILES
  fcl/DropOnInput_null_t.fcl
  fcl/DropOnInput_t.fcl
  TEST_PROPERTIES WILL_FAIL true
)

cet_test(DropOnInput_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "DropOnInput_t.fcl"
  DATAFILES
  fcl/DropOnInput_t.fcl
)

cet_test(DropOnInput_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "DropOnInput_r.fcl"
  DATAFILES
  fcl/DropOnInput_r.fcl
)

SET_TESTS_PROPERTIES(DropOnInput_null_t PROPERTIES DEPENDS Ptrmv_w)
SET_TESTS_PROPERTIES(DropOnInput_t PROPERTIES DEPENDS Ptrmv_w)
SET_TESTS_PROPERTIES(DropOnInput_r PROPERTIES DEPENDS DropOnInput_t)

########################################################################
# Old tests converted from CMS FWCore/Framework/test

# Utility script to turn an expected abort into a pass (see test
# Unscheduled_03_t below).
cet_test(test_must_abort PREBUILT NO_AUTO)

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
    TEST_ARGS -c ${test}.fcl
    DATAFILES
    fcl/${test}.fcl
    fcl/messageDefaults.fcl
    )
endforeach()

# Extra test properties
foreach(n 1;4;5;6)
  SET_TESTS_PROPERTIES(Unscheduled_0${n}_t PROPERTIES
    PASS_REGULAR_EXPRESSION
    "module for event:"
    )
endforeach()

SET_TESTS_PROPERTIES(Unscheduled_02_t PROPERTIES
  WILL_FAIL true
  )

# This test is a little more complicated because we expect it to abort,
# which is not covered by the WILL_FAIL test property.
cet_test(Unscheduled_03_t HANDBUILT
  TEST_EXEC test_must_abort
  TEST_ARGS art -c "Unscheduled_03_t.fcl"
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION
  "Assertion `handle.isValid\\\\\\\\(\\\\\\\\) == require_presence_' failed."
  DATAFILES
  fcl/Unscheduled_03_t.fcl
  fcl/messageDefaults.fcl
)
