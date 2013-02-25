# project name
add_library(test_Integration
  SHARED
  ToySource.cc
)

target_link_libraries(test_Integration
  art_Framework_Core
  ${FHICLCPP}
)

set_source_files_properties(MixFilterTestETS_module.cc
  PROPERTIES COMPILE_DEFINITIONS ART_TEST_EVENTS_TO_SKIP_CONST=0
)

set_source_files_properties(MixFilterTestETSc_module.cc
  PROPERTIES COMPILE_DEFINITIONS ART_TEST_EVENTS_TO_SKIP_CONST=1
)

set_source_files_properties(MixFilterTestNoStartEvent_module.cc
  PROPERTIES COMPILE_DEFINITIONS ART_TEST_NO_STARTEVENT
)

set_source_files_properties(MixFilterTestOldStartEvent_module.cc
  PROPERTIES COMPILE_DEFINITIONS ART_TEST_OLD_STARTEVENT
)

simple_plugin(AddIntsProducer            "module" NOINSTALL )
simple_plugin(AssnsAnalyzer              "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(AssnsProducer              "module" NOINSTALL )
simple_plugin(BareStringAnalyzer         "module" NOINSTALL )
simple_plugin(BareStringProducer         "module" NOINSTALL )
simple_plugin(DerivedPtrVectorProducer   "module" NOINSTALL )
simple_plugin(DoubleProducer             "module" NOINSTALL )
simple_plugin(DoubleTestAnalyzer         "module" NOINSTALL )
simple_plugin(DropTestParentageFaker     "module" NOINSTALL )
simple_plugin(DropTestAnalyzer           "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(FailingAnalyzer            "module" NOINSTALL )
simple_plugin(FailingProducer            "module" NOINSTALL )
simple_plugin(GeneratorTest              "source" art_Framework_IO_Sources NOINSTALL )
simple_plugin(IntProducer                "module" NOINSTALL )
simple_plugin(IntTestAnalyzer            "module" NOINSTALL )
simple_plugin(IntVectorAnalyzer          "module" NOINSTALL )
simple_plugin(IntVectorProducer          "module" NOINSTALL )
simple_plugin(MixAnalyzer                "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTest              "module" art_Framework_IO_ProductMix NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTestETS           "module" art_Framework_IO_ProductMix NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTestETSc          "module" art_Framework_IO_ProductMix NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTestNoStartEvent  "module" art_Framework_IO_ProductMix NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTestOldStartEvent "module" art_Framework_IO_ProductMix NOINSTALL USE_BOOST_UNIT )
simple_plugin(MixProducer                "module" NOINSTALL )
simple_plugin(MockClusterListAnalyzer    "module" NOINSTALL )
simple_plugin(MockClusterListProducer    "module" NOINSTALL )
simple_plugin(ProductIDGetter            "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(ProductIDGetterAnalyzer    "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(PtrListAnalyzer            "module" NOINSTALL )
simple_plugin(PtrVectorAnalyzer          "module" NOINSTALL )
simple_plugin(PtrVectorProducer          "module" NOINSTALL )
simple_plugin(PtrVectorSimpleAnalyzer    "module" NOINSTALL )
simple_plugin(PtrmvAnalyzer              "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(PtrmvProducer              "module" NOINSTALL )
simple_plugin(RandomNumberSaveTest       "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(Reconfigurable             "service" NOINSTALL )
simple_plugin(Reconfiguring              "module" NOINSTALL )
simple_plugin(SAMMetadataTest            "module" NOINSTALL )
simple_plugin(ServiceUsing               "service" NOINSTALL )
simple_plugin(SimpleDerivedAnalyzer      "module" NOINSTALL )
simple_plugin(SimpleDerivedProducer      "module" NOINSTALL )
simple_plugin(TestBitsOutput             "module" NOINSTALL )
simple_plugin(TestFilter                 "module" NOINSTALL )
simple_plugin(TestOutput                 "module" NOINSTALL )
simple_plugin(TestProvenanceDumper       "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(TestResultAnalyzer         "module" NOINSTALL )
simple_plugin(TestServiceUsingService    "module" NOINSTALL USE_BOOST_UNIT )
simple_plugin(Throwing                   "service" NOINSTALL )
simple_plugin(ToyRawFileInput            "source" test_Integration art_Framework_IO_Sources NOINSTALL USE_BOOST_UNIT )
simple_plugin(ToyRawInput                "source" test_Integration art_Framework_IO_Sources NOINSTALL USE_BOOST_UNIT )
simple_plugin(ToyRawInputTester          "module" NOINSTALL )
simple_plugin(ToyRawProductAnalyzer      "module" NOINSTALL )
simple_plugin(U_S                        "module" ALLOW_UNDERSCORES NOINSTALL )
simple_plugin(Wanted                     "service" NOINSTALL )

art_dictionary(DICTIONARY_LIBRARIES ${CETLIB} NOINSTALL)

# Common library link requirements for tests here.
set(default_test_libraries
  art_Framework_Art
  art_Framework_Services_Registry
  art_Utilities
  art_Framework_Core
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_CINTEX}
  ${ROOT_TREE}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_THREAD}
  ${ROOT_RIO}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${ROOT_REFLEX}
  ${CPPUNIT}
  -ldl
 )

# cet_test macro

# Tests
cet_test(ReflexTools_t USE_BOOST_UNIT
  LIBRARIES ${default_test_libraries}
  )

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

cet_test(SimpleDerived_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_simplederived_01a.fcl
  DATAFILES
  fcl/test_simplederived_01a.fcl
)

cet_test(SimpleDerived_01_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_simplederived_01b.fcl
  DATAFILES
  fcl/test_simplederived_01b.fcl
  TEST_PROPERTIES DEPENDS SimpleDerived_01_w
)

cet_test(SimpleDerived_02_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c SimpleDerived_02.fcl
  DATAFILES
  fcl/SimpleDerived_02.fcl
  TEST_PROPERTIES DEPENDS SimpleDerived_01_w
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
  TEST_ARGS --rethrow-all -c test_reconfig_01.fcl
  DATAFILES
  fcl/test_reconfig_01.fcl
  fcl/messageDefaults.fcl
)

cet_test(issue_0930_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c issue_0930.fcl
  DATAFILES
  fcl/issue_0930.fcl
  fcl/messageDefaults.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "LogicError BEGIN"
)

cet_test(test_FileDumperOutput.sh PREBUILT
  DATAFILES
  fcl/FileDumperOutputTest_w.fcl
  fcl/FileDumperOutputTest_r.fcl
)

cet_test(ToyRawInput_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawInput_01.fcl
  DATAFILES
  fcl/ToyRawInput_01.fcl
)

foreach(num 02 03 04)
  cet_test(ToyRawInput_t_${num} HANDBUILT
    TEST_EXEC test_must_abort
    TEST_ARGS art -c ToyRawInput_${num}.fcl
    DATAFILES
    fcl/ToyRawInput_${num}.fcl
    fcl/ToyRawInput_common.fcl
  )
endforeach()


SET_TESTS_PROPERTIES(ToyRawInput_t_02 PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Assertion `outR \\|\\| inR' failed"
)

SET_TESTS_PROPERTIES(ToyRawInput_t_03 ToyRawInput_t_04
  PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Assertion `outSR \\|\\| inSR' failed"
)

foreach(num 05 06 07 08 09 10 11 12)
  cet_test(ToyRawInput_t_${num} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ToyRawInput_${num}.fcl
    DATAFILES
    fcl/ToyRawInput_${num}.fcl
    fcl/ToyRawInput_common.fcl
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
  TEST_ARGS --rethrow-all -c ToyRawInput_w.fcl
  DATAFILES
  fcl/ToyRawInput_w.fcl
  fcl/ToyRawInput_wr_f1.fcl
  fcl/ToyRawInput_common.fcl
)

cet_test(ToyRawInput_r1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawInput_r1.fcl
  DATAFILES
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawInput_w
)

cet_test(ToyRawInput_r2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawInput_r2.fcl
  DATAFILES
  fcl/ToyRawInput_r2.fcl
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawInput_w
)

cet_test(ToyRawInput_r3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawInput_r3.fcl
  DATAFILES
  fcl/ToyRawInput_r3.fcl
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawInput_w
)

cet_test(ToyRawFileInput_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawFileInput_w.fcl
  DATAFILES
  fcl/ToyRawFileInput_data.fcl
  fcl/ToyRawFileInput_w.fcl
  fcl/ToyRawInput_w.fcl
  fcl/ToyRawInput_wr_f1.fcl
  fcl/ToyRawInput_common.fcl
)

cet_test(ToyRawFileInput_r1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawFileInput_r1.fcl
  DATAFILES
  fcl/ToyRawFileInput_r1.fcl
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawFileInput_w
)

cet_test(ToyRawFileInput_r2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawFileInput_r2.fcl
  DATAFILES
  fcl/ToyRawFileInput_r2.fcl
  fcl/ToyRawInput_r2.fcl
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawFileInput_w
)

cet_test(ToyRawFileInput_r3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyRawFileInput_r3.fcl
  DATAFILES
  fcl/ToyRawFileInput_r3.fcl
  fcl/ToyRawInput_r3.fcl
  fcl/ToyRawInput_r1.fcl
  TEST_PROPERTIES DEPENDS ToyRawFileInput_w
)

cet_test(BareString_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c BareString_w.fcl
  DATAFILES
  fcl/BareString_w.fcl
)

cet_test(BareString_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c BareString_r.fcl
  DATAFILES
  fcl/BareString_r.fcl
  TEST_PROPERTIES DEPENDS BareString_w
)


# Write the data file required for the other mixing tests.
cet_test(ProductMix_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c "ProductMix_w.fcl"
  DATAFILES
  fcl/ProductMix_w.fcl
)

# Mix the events from ProductMix_w, analyzing the results and
# writing an output file.
cet_test(ProductMix_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with primary event #2 having no secondaries to mix. No analysis is done
# nor output written.
cet_test(ProductMix_r1a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1a.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1a.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with multiple secondary input files (actually, the same file specified
# twice).
cet_test(ProductMix_r1b HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1b.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1b.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrappring and no eventsToSkip().
cet_test(ProductMix_r1c1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1c.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

SET_TESTS_PROPERTIES(ProductMix_r1c1
  PROPERTIES
  PASS_REGULAR_EXPRESSION "TrigReport Events total = 400 passed = 400 failed = 0.*MixingInputWrap      -w MixFilterTest[A-Za-z:]*                       1"
  )

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrappring and eventsToSkip().
cet_test(ProductMix_r1c2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1c2.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  fcl/ProductMix_r1c2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrappring and eventsToSkip() const.
cet_test(ProductMix_r1c3 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1c3.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  fcl/ProductMix_r1c3.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with no startEvent function.
cet_test(ProductMix_r1d1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1d1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d1.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with an old-signature startEvent function.
cet_test(ProductMix_r1d2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1d2.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

SET_TESTS_PROPERTIES(
  ProductMix_r1c2
  ProductMix_r1c3
  PROPERTIES
  PASS_REGULAR_EXPRESSION "TrigReport Events total = 400 passed = 400 failed = 0.*MixingInputWrap      -w MixFilterTest[A-Za-z:]*                       2"
  )

# Mix the events from ProductMix_w, testing that we correctly detect an
# inappropriate attempt to dereference a Ptr.
cet_test(ProductMix_r1d HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r1d.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Read the output from ProductMix_r1a running the analysis to ensure that
# everything works as read from an output file written post-mixing.
cet_test(ProductMix_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductMix_r2.fcl"
  DATAFILES
  fcl/ProductMix_r2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_r1
)

cet_test(Ptrmv_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "Ptrmv_w.fcl"
  DATAFILES
  fcl/Ptrmv_w.fcl
)

cet_test(Ptrmv_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "Ptrmv_r.fcl"
  DATAFILES
  fcl/Ptrmv_r.fcl
  TEST_PROPERTIES DEPENDS Ptrmv_w
)

cet_test(ProductIDGetter_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProductIDGetter_t.fcl"
  DATAFILES
  fcl/ProductIDGetter_t.fcl
  )

cet_test(Assns_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "Assns_w.fcl"
  DATAFILES
  fcl/Assns_w.fcl
)

foreach(num 1 2 3)
  cet_test(Assns_r${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all -c "Assns_r${num}.fcl"
    DATAFILES
    fcl/Assns_r_inc.fcl
    fcl/Assns_r${num}.fcl
    TEST_PROPERTIES DEPENDS Assns_w
  )
endforeach()

# Use the file from Ptrmv_w to test DropOnInput and outputCommand drops.
#
# 1. Drop Ptr, keep base MapVector.
# 2, Keep Ptr, drop base MapVector.
# 3. Drop both.
# 4. Keep both.
foreach(num RANGE 1 4)
  # Test drop on input.
  cet_test(DropOnInput_t_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all
    -c "DropOnInput_t_0${num}.fcl"
    -s "../Ptrmv_w.d/out.root"
    --output "out.root"
    DATAFILES
    fcl/DropOnInput_t_01.fcl
    fcl/DropOnInput_t_0${num}.fcl
    TEST_PROPERTIES DEPENDS Ptrmv_w
    )

  cet_test(DropOnInput_r_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all
    -c "DropOnInput_r_0${num}.fcl"
    -s "../DropOnInput_t_0${num}.d/out.root"
    DATAFILES
    fcl/DropOnInput_t_01.fcl
    fcl/DropOnInput_t_0${num}.fcl
    fcl/DropOnInput_r_0${num}.fcl
    TEST_PROPERTIES DEPENDS DropOnInput_t_0${num}
    )

  # Test drop on output with input file.
  cet_test(DropOnOutput_wA_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all -c "DropOnOutput_wA_0${num}.fcl"
    DATAFILES
    fcl/DropOnOutput_wA_01.fcl
    fcl/DropOnOutput_wA_0${num}.fcl
    TEST_PROPERTIES DEPENDS Ptrmv_w
    )

  cet_test(DropOnOutput_rA_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all
    -c "DropOnOutput_r_0${num}.fcl"
    -s "../DropOnOutput_wA_0${num}.d/out.root"
    DATAFILES
    fcl/DropOnOutput_r_01.fcl
    fcl/DropOnOutput_r_0${num}.fcl
    TEST_PROPERTIES DEPENDS DropOnOutput_wA_0${num}
    )

  # Test drop on output with in-job generated products.
  cet_test(DropOnOutput_wB_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all -c "DropOnOutput_wB_0${num}.fcl"
    DATAFILES
    fcl/Ptrmv_w.fcl
    fcl/DropOnOutput_wB_01.fcl
    fcl/DropOnOutput_wB_0${num}.fcl
    )

  cet_test(DropOnOutput_rB_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS --rethrow-all
    -c "DropOnOutput_r_0${num}.fcl"
    -s "../DropOnOutput_wB_0${num}.d/out.root"
    DATAFILES
    fcl/DropOnOutput_r_01.fcl
    fcl/DropOnOutput_r_0${num}.fcl
    TEST_PROPERTIES DEPENDS DropOnOutput_wB_0${num}
    )
endforeach()

# Write a data file containing stored products and stored random number
# states for subsequent tests.
cet_test(RandomNumberTestEventSave_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestEventSave_w.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_w.fcl
)

# Test the ability to restore random number states from data product.
cet_test(RandomNumberTestEventSave_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestEventSave_r.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestEventSave_w
)

# Test that we get numbers out of sync. when we do *not* restore the
# state from data product
cet_test(RandomNumberTestEventSave_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestEventSave_r2.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestEventSave_r2.fcl
  TEST_PROPERTIES WILL_FAIL true
  DEPENDS RandomNumberTestEventSave_w
)

# Write the state file terminating normally after 9 events.
cet_test(RandomNumberTestFileSave_wA HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestFileSave_wA.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_wA.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestEventSave_w
)

# Read the normal-termination state file to process the 10th event.
cet_test(RandomNumberTestFileSave_rA HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestFileSave_rA.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_rA.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestFileSave_wA
)

# Write the state file terminating abnormally in event #10.
cet_test(RandomNumberTestFileSave_wB HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestFileSave_wB.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_wB.fcl
  TEST_PROPERTIES WILL_FAIL true
  DEPENDS RandomNumberTestEventSave_w
)

# Read the abnormal-termination state file to process the 10th event.
cet_test(RandomNumberTestFileSave_rB HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "RandomNumberTestFileSave_rB.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_rB.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestFileSave_wB
)

# Verify the ProvenanceChecker operation with a non-trivial event
# structure.
cet_test(ProvenanceChecker_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProvenanceChecker_t.fcl"
  DATAFILES
  fcl/Ptrmv_w.fcl
  fcl/ProvenanceChecker_t.fcl
)

# Test the ProvenanceDumper module template (write and check).
cet_test(ProvenanceDumper_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProvenanceDumper_w.fcl"
  DATAFILES
  fcl/FileDumperOutputTest_w.fcl
  fcl/ProvenanceDumper_w.fcl
)

# Test the ProvenanceDumper module template (read and check).
foreach(num 1 2 3 4)
  cet_test(ProvenanceDumper_r${num} HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS --rethrow-all -c "ProvenanceDumper_r${num}.fcl"
  DATAFILES
  fcl/FileDumperOutputTest_r.fcl
  fcl/ProvenanceDumper_r1.fcl
  fcl/ProvenanceDumper_r${num}.fcl # Should be ignored as dup. as appropriate.
  TEST_PROPERTIES DEPENDS ProvenanceDumper_w
  )
endforeach()

cet_test(PtrVectorFastClone_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "PtrVectorFastClone_w.fcl"
  DATAFILES
  fcl/PtrVectorFastClone_w.fcl
  TEST_PROPERTIES DEPENDS Ptrmv_w
)

cet_test(AssnsFastClone_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "AssnsFastClone_w.fcl"
  DATAFILES
  fcl/AssnsFastClone_w.fcl
  TEST_PROPERTIES DEPENDS Assns_w
)

cet_test(ServiceUsingService_01_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "ServiceUsingService_01.fcl"
  DATAFILES
  fcl/ServiceUsingService_01.fcl
)

cet_test(ServiceUsingService_02_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c "ServiceUsingService_02.fcl"
  DATAFILES
  fcl/test_reconfig_01.fcl
  fcl/ServiceUsingService_02.fcl
)

cet_test(SAM_metadata HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "SAMMetadata_w.fcl"
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-data-tier "The one with the thickest frosting"
  DATAFILES
  fcl/SAMMetadata_w.fcl
)

cet_test(SAM_metadata_verify HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -s ../SAM_metadata.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
)

SET_TESTS_PROPERTIES(SAM_metadata_verify PROPERTIES
  PASS_REGULAR_EXPRESSION
  "\n\n0: applicationFamily  Ethel\n0: applicationVersion v0.00.01a\n0: fileType           MC\n0: process_name       SAMMetadataW\n0: testMetadata       success!\n0: dataTier           The one with the thickest frosting\n0: streamName         o1\n-"
)

cet_test(GeneratorSource_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "GeneratorSource_t.fcl"
  DATAFILES
  fcl/GeneratorSource_t.fcl
  )


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
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION
  "Assertion `handle.isValid\\\\\\\\(\\\\\\\\) == require_presence_' failed."
  DATAFILES
  fcl/Unscheduled_03_t.fcl
  fcl/messageDefaults.fcl
)
