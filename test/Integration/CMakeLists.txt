set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

# project name
art_make_library(SOURCE
  ToySource.cc
  LIBRARIES art_Framework_IO_Sources
  art_Framework_Core
  fhiclcpp
  NO_INSTALL
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

simple_plugin(AddIntsProducer             "module"  NO_INSTALL )
simple_plugin(AssnsAnalyzer               "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(AssnsProducer               "module"  NO_INSTALL )
simple_plugin(BareStringAnalyzer          "module"  NO_INSTALL )
simple_plugin(BareStringProducer          "module"  NO_INSTALL )
simple_plugin(CompressedIntProducer       "module"  NO_INSTALL )
simple_plugin(CompressedIntTestAnalyzer   "module"  NO_INSTALL )
simple_plugin(DeferredIsReadyWithAssnsAnalyzer "module"  NO_INSTALL )
simple_plugin(DeferredIsReadyWithAssnsProducer "module"  NO_INSTALL )
simple_plugin(DerivedPtrVectorProducer    "module"  NO_INSTALL )
simple_plugin(DoubleProducer              "module"  NO_INSTALL )
simple_plugin(DoubleTestAnalyzer          "module"  NO_INSTALL )
simple_plugin(DropTestAnalyzer            "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(DropTestParentageFaker      "module"  NO_INSTALL )
simple_plugin(FailingAnalyzer             "module"  NO_INSTALL )
simple_plugin(FailingProducer             "module"  NO_INSTALL )
simple_plugin(UnputtingProducer           "module"  NO_INSTALL )
simple_plugin(FlushingGenerator           "source"  NO_INSTALL )
simple_plugin(FlushingGeneratorTest       "module"  NO_INSTALL )
simple_plugin(FlushingGeneratorTestFilter "module"  NO_INSTALL )
simple_plugin(GeneratorTest               "source"  NO_INSTALL )
simple_plugin(InFlightConfiguration       "service" NO_INSTALL USE_BOOST_UNIT art_Framework_Services_Registry art_Framework_Services_UserInteraction art_Framework_Services_System_PathSelection_service )
simple_plugin(IntProducer                 "module"  NO_INSTALL )
simple_plugin(IntTestAnalyzer             "module"  NO_INSTALL )
simple_plugin(IntVectorAnalyzer           "module"  NO_INSTALL )
simple_plugin(IntVectorProducer           "module"  NO_INSTALL )
simple_plugin(MixAnalyzer                 "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(MixFilterTest               "module"  NO_INSTALL USE_BOOST_UNIT art_Framework_IO_ProductMix )
simple_plugin(MixFilterTestETS            "module"  NO_INSTALL USE_BOOST_UNIT art_Framework_IO_ProductMix )
simple_plugin(MixFilterTestETSc           "module"  NO_INSTALL USE_BOOST_UNIT art_Framework_IO_ProductMix )
simple_plugin(MixFilterTestNoStartEvent   "module"  NO_INSTALL USE_BOOST_UNIT art_Framework_IO_ProductMix )
simple_plugin(MixFilterTestOldStartEvent  "module"  NO_INSTALL USE_BOOST_UNIT art_Framework_IO_ProductMix )
simple_plugin(MixProducer                 "module"  NO_INSTALL )
simple_plugin(MockClusterListAnalyzer     "module"  NO_INSTALL )
simple_plugin(MockClusterListProducer     "module"  NO_INSTALL )
simple_plugin(PausingAnalyzer             "module"  NO_INSTALL )
simple_plugin(ProductIDGetter             "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(ProductIDGetterNoPut        "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(ProductIDGetterAnalyzer     "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(PtrListAnalyzer             "module"  NO_INSTALL )
simple_plugin(PtrVectorSimpleAnalyzer     "module"  NO_INSTALL )
simple_plugin(PtrmvAnalyzer               "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(PtrmvProducer               "module"  NO_INSTALL )
simple_plugin(RandomNumberSaveTest        "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(Reconfigurable              "service" NO_INSTALL )
simple_plugin(Reconfiguring               "module"  NO_INSTALL )
simple_plugin(RunProductAnalyzer          "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(RunProductProducer          "module" NO_INSTALL)
simple_plugin(RunSubRunProductProducerNoPut "module"  NO_INSTALL )
simple_plugin(SAMMetadataTest             "module"  NO_INSTALL art_Framework_Services_System_FileCatalogMetadata_service )
simple_plugin(ServiceUsing                "service" NO_INSTALL )
simple_plugin(SimpleDerivedAnalyzer       "module"  NO_INSTALL )
simple_plugin(SimpleDerivedProducer       "module"  NO_INSTALL )
simple_plugin(TH1DataProducer             "module"  NO_INSTALL test_TestObjects)
simple_plugin(TestAnalyzerSelect          "module"  NO_INSTALL )
simple_plugin(TestBitsOutput              "module"  NO_INSTALL )
simple_plugin(TestFilter                  "module"  NO_INSTALL )
simple_plugin(TestOutput                  "module"  NO_INSTALL )
simple_plugin(TestProvenanceDumper        "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(TestResultAnalyzer          "module"  NO_INSTALL )
simple_plugin(TestServiceUsingService     "module"  NO_INSTALL USE_BOOST_UNIT )
simple_plugin(TestSimpleMemoryCheckProducer "module"  NO_INSTALL )
simple_plugin(TestTimeTrackerProducer     "module"  NO_INSTALL )
simple_plugin(TestTimeTrackerFilter       "module"  NO_INSTALL )
simple_plugin(TestTimeTrackerAnalyzer     "module"  NO_INSTALL )
simple_plugin(Throwing                    "service" NO_INSTALL )
simple_plugin(ToyProductAnalyzer          "module"  NO_INSTALL )
simple_plugin(ToyProductProducer          "module"  NO_INSTALL )
simple_plugin(ToyProductProducerMultiput  "module"  NO_INSTALL )
simple_plugin(ToyRawFileInput             "source"  NO_INSTALL USE_BOOST_UNIT test_Integration )
simple_plugin(ToyRawInput                 "source"  NO_INSTALL USE_BOOST_UNIT test_Integration )
simple_plugin(ToyRawInputTester           "module"  NO_INSTALL )
simple_plugin(ToyRawProductAnalyzer       "module"  NO_INSTALL )
simple_plugin(U_S                         "module"  NO_INSTALL ALLOW_UNDERSCORES )
simple_plugin(ValidHandleTester           "module"  NO_INSTALL )
simple_plugin(Wanted                      "service" NO_INSTALL )

art_dictionary(DICTIONARY_LIBRARIES cetlib NO_INSTALL)

cet_make_exec(dump_file_index
  SOURCE dump_file_index.cc
  NO_INSTALL
  LIBRARIES
  art_Framework_IO_Root
  )

cet_make_exec(dump_file_contributors
  SOURCE dump_file_contributors.cc
  NO_INSTALL
  LIBRARIES
  art_Framework_IO_Root
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_RIO}
  )

# cet_test macro

cet_test(EventSelectorFromFile_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config EventSelectorFromFile_w.fcl
  DATAFILES
  fcl/EventSelectorFromFile_w.fcl
)

cet_test(EventSelectorFromFile_r1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config EventSelectorFromFile_r1.fcl
  DATAFILES
  fcl/EventSelectorFromFile_r1.fcl
  TEST_PROPERTIES DEPENDS EventSelectorFromFile_w
)

cet_test(EventSelectorFromFile_r2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config EventSelectorFromFile_r2.fcl
  DATAFILES
  fcl/EventSelectorFromFile_r2.fcl
  TEST_PROPERTIES DEPENDS EventSelectorFromFile_w
)

cet_test(test_dropAllEventsSubruns_t.sh PREBUILT
  DATAFILES
  fcl/dropAllEvents_t.fcl
  fcl/dropAllEventsSubruns_t1.fcl
  fcl/dropAllEventsSubruns_t2.fcl
  fcl/dropAllEvents_r_t.fcl
  fcl/dropAllEventsSubruns_r_t1.fcl
  fcl/dropAllEventsSubruns_r_t2.fcl
  test_dropAllEventsSubruns_verify.cxx
)

cet_test(test_runSubRunNoPut_t.sh PREBUILT
  DATAFILES
  fcl/runSubRunNoPut_t.fcl
  )

cet_test(test_simple_01_t.sh PREBUILT
  DATAFILES
  fcl/test_simple_01.fcl
  fcl/test_simple_01r.fcl
  fcl/messageDefaults.fcl
  test_simple_01_verify.cxx
)

cet_test(test_view_01_t.sh PREBUILT
  DATAFILES
  fcl/test_view_01a.fcl
  fcl/test_view_01b.fcl
  fcl/test_simple_01r.fcl
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

cet_test(ptr_list_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config ptr_list_01.fcl
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

cet_test(test_unputtingProducer_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_unputtingProducer.fcl
  DATAFILES
  fcl/test_unputtingProducer.fcl
  fcl/messageDefaults.fcl
  TEST_PROPERTIES WILL_FAIL TRUE
)

cet_test(issue_0923_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_0923b.fcl --debug-config config.out
  DATAFILES
  fcl/issue_0923a.fcl
  fcl/issue_0923b.fcl
  TEST_PROPERTIES WILL_FAIL TRUE
)

cet_test(issue_0923_r HANDBUILT
  TEST_EXEC diff
  TEST_ARGS -u ${CMAKE_CURRENT_SOURCE_DIR}/issue_0923_ref.txt ../issue_0923_w.d/config.out
  TEST_PROPERTIES DEPENDS issue_0923_w
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

cet_test(FileDumperOutput_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FileDumperOutputTest_w.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Total products \\(present, not present\\): 4 \\(4, 0\\)\\."
  DATAFILES
  fcl/FileDumperOutputTest_w.fcl
)

cet_test(FileDumperOutput_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FileDumperOutputTest_r.fcl
  DATAFILES
  fcl/FileDumperOutputTest_r.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Total products \\(present, not present\\): 4 \\(3, 1\\)\\."
  DEPENDS FileDumperOutput_w
)

cet_test(ToyProductProducerAnalyzer_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c toyProductProducerAnalyzer_t.fcl
  DATAFILES
  fcl/toyProductProducerAnalyzer_t.fcl
  )

cet_test(RunProductAggregation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c runProductAggregation_w.fcl
  DATAFILES
  fcl/runProductAggregation_w.fcl
  )

cet_test(RunProductAggregation_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c runProductAggregation_r.fcl
  -s "../RunProductAggregation_w.d/out.root"
  DATAFILES
  fcl/runProductAggregation_r.fcl
  TEST_PROPERTIES DEPENDS RunProductAggregation_w
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

cet_test_assertion("outR \\\\|\\\\| inR"
  ToyRawInput_t_02
  )
cet_test_assertion("outSR \\\\|\\\\| inSR"
  ToyRawInput_t_03
  ToyRawInput_t_04
  )

foreach(num 01 02 03)
  cet_test(ToyProductProducerMultiput_t_${num} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ToyProductProducerMultiput_t_${num}.fcl
    DATAFILES
    fcl/ToyProductProducerMultiput_t_${num}.fcl
    TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION "Attempt to put multiple products with the"
    PASS_REGULAR_EXPRESSION "following description onto the"
    )
endforeach()


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

cet_test(ValidHandleTester HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ValidHandleTester.fcl
  DATAFILES
  fcl/ValidHandleTester.fcl
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
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with primary event #2 having no secondaries to mix. No analysis is done
# nor output written.
cet_test(ProductMix_r1a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1a.fcl"
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
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1b.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1b.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrapping and no eventsToSkip().
cet_test(ProductMix_r1c1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1c.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
  PASS_REGULAR_EXPRESSION "TrigReport Events total = 400 passed = 400 failed = 0.*MixingInputWrap      -w MixFilterTest[A-Za-z:]*                       1"
)

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrapping, respondToXXX() functions and
# eventsToSkip().
cet_test(ProductMix_r1c2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1c2.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  fcl/ProductMix_r1c2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with secondary input file wrapping and eventsToSkip() const.
cet_test(ProductMix_r1c3 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1c3.fcl"
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
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1d1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d1.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with an old-signature startEvent function.
cet_test(ProductMix_r1d2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1d2.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that the secondary event
# selection is as specified. Order is:
#   SEQUENTIAL
#   RANDOM_REPLACE
#   RANDOM_LIM_REPLACE (require no dupes within a primary)
#   RANDOM_LIM_REPLACE (require dupes across a job).
#   RANDOM_NO_REPLACE
foreach(test 1 2 3 4 5)
  cet_test(ProductMix_r1e${test} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "ProductMix_r1e${test}.fcl"
    DATAFILES
    fcl/ProductMix_r1.fcl
    fcl/ProductMix_r1e.fcl
    fcl/ProductMix_r1e${test}.fcl
    TEST_PROPERTIES DEPENDS ProductMix_w
    )
endforeach()

# Mix the events from ProductMix_w, obtaining the filenames from a
# detail-object-provided function rather than from fileNames_.
cet_test(ProductMix_r1f1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1f1.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1c.fcl
  fcl/ProductMix_r1f1.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Mix the events from ProductMix_w, testing that things behave properly
# with the compactMissingProducts option selected.
cet_test(ProductMix_r1g HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1g.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1g.fcl
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
  TEST_ARGS -- --rethrow-all -c "ProductMix_r1d.fcl"
  DATAFILES
  fcl/ProductMix_r1.fcl
  fcl/ProductMix_r1d.fcl
  TEST_PROPERTIES DEPENDS ProductMix_w
)

# Read the output from ProductMix_r1a running the analysis to ensure that
# everything works as read from an output file written post-mixing.
cet_test(ProductMix_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductMix_r2.fcl"
  DATAFILES
  fcl/ProductMix_r2.fcl
  TEST_PROPERTIES DEPENDS ProductMix_r1
)

cet_test(Ptrmv_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "Ptrmv_w.fcl"
  DATAFILES
  fcl/Ptrmv_w.fcl
)

cet_test(Ptrmv_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "Ptrmv_r.fcl"
  DATAFILES
  fcl/Ptrmv_r.fcl
  TEST_PROPERTIES DEPENDS Ptrmv_w
)

cet_test(ProductIDGetter_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetter_t.fcl"
  DATAFILES
  fcl/ProductIDGetter_t.fcl
  )

cet_test(ProductIDGetterNoPut_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t.fcl"
  DATAFILES
  fcl/ProductIDGetterNoPut_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'"
  )

cet_test(ProductIDGetterNoPut_t_01a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t_01.fcl"
  DATAFILES
  fcl/ProductIDGetterNoPut_t_01.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'"
  )

cet_test(ProductIDGetterNoPut_t_01b HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all --errorOnFailureToPut=false -c "ProductIDGetterNoPut_t_01.fcl"
  DATAFILES
  fcl/ProductIDGetterNoPut_t_01.fcl
  )

cet_test(ProductIDGetterNoPut_t_02a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t_02.fcl"
  DATAFILES
  fcl/ProductIDGetterNoPut_t_02.fcl
  )

cet_test(ProductIDGetterNoPut_t_02b HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all --errorOnFailureToPut=true -c "ProductIDGetterNoPut_t_02.fcl"
  DATAFILES
  fcl/ProductIDGetterNoPut_t_02.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'"
  )

cet_test(Assns_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "Assns_w.fcl"
  DATAFILES
  fcl/Assns_w.fcl
)

foreach(num 1 2 3)
  cet_test(Assns_r${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all -c "Assns_r${num}.fcl"
    DATAFILES
    fcl/Assns_r_inc.fcl
    fcl/Assns_r${num}.fcl
    TEST_PROPERTIES DEPENDS Assns_w
    )
endforeach()

cet_test(DeferredIsReadyWithAssns_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config "DeferredIsReadyWithAssns_t.fcl"
  DATAFILES
  fcl/DeferredIsReadyWithAssns_t.fcl
  )

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
    TEST_ARGS -- --rethrow-all
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
    TEST_ARGS -- --rethrow-all
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
    TEST_ARGS -- --rethrow-all -c "DropOnOutput_wA_0${num}.fcl"
    DATAFILES
    fcl/DropOnOutput_wA_01.fcl
    fcl/DropOnOutput_wA_0${num}.fcl
    TEST_PROPERTIES DEPENDS Ptrmv_w
    )

  cet_test(DropOnOutput_rA_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all
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
    TEST_ARGS -- --rethrow-all -c "DropOnOutput_wB_0${num}.fcl"
    DATAFILES
    fcl/Ptrmv_w.fcl
    fcl/DropOnOutput_wB_01.fcl
    fcl/DropOnOutput_wB_0${num}.fcl
    )

  cet_test(DropOnOutput_rB_0${num} HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --rethrow-all
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
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestEventSave_w.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_w.fcl
)

# Test the ability to restore random number states from data product.
cet_test(RandomNumberTestEventSave_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestEventSave_r.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestEventSave_w
)

# Test that we get numbers out of sync. when we do *not* restore the
# state from data product
cet_test(RandomNumberTestEventSave_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestEventSave_r2.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestEventSave_r2.fcl
  TEST_PROPERTIES WILL_FAIL true
  DEPENDS RandomNumberTestEventSave_w
)

# Write the state file terminating normally after 9 events.
cet_test(RandomNumberTestFileSave_wA HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestFileSave_wA.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_wA.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestEventSave_w
)

# Read the normal-termination state file to process the 10th event.
cet_test(RandomNumberTestFileSave_rA HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestFileSave_rA.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_rA.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestFileSave_wA
)

# Write the state file terminating abnormally in event #10.
cet_test(RandomNumberTestFileSave_wB HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestFileSave_wB.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_wB.fcl
  TEST_PROPERTIES WILL_FAIL true
  DEPENDS RandomNumberTestEventSave_w
)

# Read the abnormal-termination state file to process the 10th event.
cet_test(RandomNumberTestFileSave_rB HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RandomNumberTestFileSave_rB.fcl"
  DATAFILES
  fcl/RandomNumberTestEventSave_r.fcl
  fcl/RandomNumberTestFileSave_rB.fcl
  TEST_PROPERTIES DEPENDS RandomNumberTestFileSave_wB
)

# Verify the ProvenanceChecker operation with a non-trivial event
# structure.
cet_test(ProvenanceChecker_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProvenanceChecker_t.fcl"
  DATAFILES
  fcl/Ptrmv_w.fcl
  fcl/ProvenanceChecker_t.fcl
)

# Test the ProvenanceDumper module template (write and check).
cet_test(ProvenanceDumper_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProvenanceDumper_w.fcl"
  DATAFILES
  fcl/FileDumperOutputTest_w.fcl
  fcl/ProvenanceDumper_w.fcl
)

# Test the ProvenanceDumper module template (read and check).
foreach(num 1 2 3 4)
  cet_test(ProvenanceDumper_r${num} HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProvenanceDumper_r${num}.fcl"
  DATAFILES
  fcl/FileDumperOutputTest_r.fcl
  fcl/ProvenanceDumper_r1.fcl
  fcl/ProvenanceDumper_r${num}.fcl # Should be ignored as dup. as appropriate.
  TEST_PROPERTIES DEPENDS ProvenanceDumper_w
  )
endforeach()

cet_test(PtrVectorFastClone_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c "PtrVectorFastClone_w.fcl"
  DATAFILES
  fcl/PtrVectorFastClone_w.fcl
  TEST_PROPERTIES DEPENDS Ptrmv_w
)

cet_test(AssnsFastClone_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c "AssnsFastClone_w.fcl"
  DATAFILES
  fcl/AssnsFastClone_w.fcl
  TEST_PROPERTIES DEPENDS Assns_w
)

cet_test(ServiceUsingService_01_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c "ServiceUsingService_01.fcl"
  DATAFILES
  fcl/ServiceUsingService_01.fcl
)

cet_test(ServiceUsingService_02_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c "ServiceUsingService_02.fcl"
  DATAFILES
  fcl/test_reconfig_01.fcl
  fcl/ServiceUsingService_02.fcl
)

cet_test(SimpleMemoryCheck_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c simpleMemoryCheck.fcl
  DATAFILES
  fcl/simpleMemoryCheck.fcl
  fcl/messageDefaults.fcl
  )

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  cet_test(MemoryTracker_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c memoryTracker.fcl
    DATAFILES
    fcl/memoryTracker.fcl
    fcl/messageDefaults.fcl
    REF "${CMAKE_CURRENT_SOURCE_DIR}/MemoryTracker_t-ref.txt"
    )
endif()

cet_test(TimingService_issue_2979_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_2979.fcl
  DATAFILES
  fcl/issue_2979.fcl
)

cet_test(TimeTracker_issue_3598_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_3598_t1.fcl
  DATAFILES fcl/issue_3598_t1.fcl
  REF "${CMAKE_CURRENT_SOURCE_DIR}/TimeTracker_issue_3598_t1-ref.txt"
  )

cet_test(TimeTracker_issue_3598_t2 HANDBUILT TEST_EXEC art TEST_ARGS -c issue_3598_t2.fcl DATAFILES fcl/issue_3598_t2.fcl)

cet_test(TimeTracker_issue_3598_t3 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_3598_t3.fcl
  DATAFILES fcl/issue_3598_t3.fcl
  REF "${CMAKE_CURRENT_SOURCE_DIR}/TimeTracker_issue_3598_t3-ref.txt"
  )

cet_test(SAM_metadata HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "SAMMetadata_w.fcl"
  --sam-application-family "Ethel"
  --sam-application-version "v0.00.01a"
  --sam-file-type "MC"
  --sam-run-type "MCChallenge"
  --sam-data-tier "The one with the thickest frosting"
  --sam-group "MyGang"
  DATAFILES
  fcl/SAMMetadata_w.fcl
)

set(date_regex "[0-9][0-9][0-9][0-9]-[01][0-9]-[0-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9](,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9])?")

# Pipe the output of sam_metadata_dumper through the following perl
# command to get a suitable regex:
#
# perl -wne 'BEGIN { print "^"; }; chomp; s&([\[\]\.])&\\\\${1}&g; s&\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:,\d{9})?&\${date_regex}&g; s&"&\\"&g; print "$_\\n"; END { print "\$\n"; }'

cet_test(SAM_metadata_verify_hr HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -H -s ../SAM_metadata.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
  REF "${CMAKE_CURRENT_SOURCE_DIR}/SAM_metadata_verify_hr-ref.txt"
  )

cet_test(SAM_metadata_verify_JSON HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -s ../SAM_metadata.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
  REF "${CMAKE_CURRENT_SOURCE_DIR}/SAM_metadata_verify_JSON-ref.txt"
  )

cet_test(SAM_metadataFromInput_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c SAMMetadata_r_passthrough1.fcl
  --sam-inherit-metadata
  --sam-data-tier "Chummy"
  -s ../SAM_metadata.d/out.root
  -o out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
  DATAFILES
  fcl/SAMMetadata_r_passthrough1.fcl
  )

cet_test(SAM_metadataFromInput_01_verify HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -H -s ../SAM_metadataFromInput_01.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadataFromInput_01
  PASS_REGULAR_EXPRESSION
  "^\nFile catalog metadata from file \\.\\./SAM_metadataFromInput_01\\.d/out\\.root:\n\n 1: process_name        \"SAMMetadataPassthrough1\"\n 2: file_type           \"MC\"\n 3: run_type            \"MCChallenge\"\n 4: data_tier           \"Chummy\"\n 5: data_stream         \"o1\"\n 6: file_format         \"artroot\"\n 7: file_format_era     \"ART_2011a\"\n 8: file_format_version 8\n 9: start_time          \"${date_regex}\"\n10: end_time            \"${date_regex}\"\n11: runs                \\[ \\[ 1, 0, \"MCChallenge\" \\] \\]\n12: event_count         1\n13: first_event         \\[ 1, 0, 1 \\]\n14: last_event          \\[ 1, 0, 1 \\]\n15: parents             \\[ \"out\\.root\" \\]\n-------------------------------\n$"
  )

cet_test(SAM_metadataFromInput_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c SAMMetadata_r_passthrough2.fcl
  --sam-inherit-run-type
  --sam-data-tier "Peachy"
  -s ../SAM_metadata.d/out.root
  -o out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
  DATAFILES
  fcl/SAMMetadata_r_passthrough2.fcl
  )

cet_test(SAM_metadataFromInput_02_verify HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -H -s ../SAM_metadataFromInput_02.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadataFromInput_02
  PASS_REGULAR_EXPRESSION
  "^\nFile catalog metadata from file \\.\\./SAM_metadataFromInput_02\\.d/out\\.root:\n\n 1: file_type           \"Pescadero\"\n 2: process_name        \"SAMMetadataPassthrough2\"\n 3: run_type            \"MCChallenge\"\n 4: data_tier           \"Peachy\"\n 5: data_stream         \"o1\"\n 6: file_format         \"artroot\"\n 7: file_format_era     \"ART_2011a\"\n 8: file_format_version 8\n 9: start_time          \"${date_regex}\"\n10: end_time            \"${date_regex}\"\n11: runs                \\[ \\[ 1, 0, \"MCChallenge\" \\] \\]\n12: event_count         1\n13: first_event         \\[ 1, 0, 1 \\]\n14: last_event          \\[ 1, 0, 1 \\]\n15: parents             \\[ \"out\\.root\" \\]\n-------------------------------\n$"
  )

cet_test(SAM_metadataFromInput_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c SAMMetadata_r_passthrough3.fcl
  --sam-inherit-file-type
  --sam-run-type "Trot"
  --sam-data-tier "Grapey"
  -s ../SAM_metadata.d/out.root
  -o out.root
  TEST_PROPERTIES DEPENDS SAM_metadata
  DATAFILES
  fcl/SAMMetadata_r_passthrough3.fcl
  )

cet_test(SAM_metadataFromInput_03_verify HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -H -s ../SAM_metadataFromInput_03.d/out.root
  TEST_PROPERTIES DEPENDS SAM_metadataFromInput_03
  PASS_REGULAR_EXPRESSION
  "^\nFile catalog metadata from file \\.\\./SAM_metadataFromInput_03\\.d/out\\.root:\n\n 1: run_type            \"Trot\"\n 2: process_name        \"SAMMetadataPassthrough3\"\n 3: file_type           \"MC\"\n 4: data_tier           \"Grapey\"\n 5: data_stream         \"o1\"\n 6: file_format         \"artroot\"\n 7: file_format_era     \"ART_2011a\"\n 8: file_format_version 8\n 9: start_time          \"${date_regex}\"\n10: end_time            \"${date_regex}\"\n11: runs                \\[ \\[ 1, 0, \"Trot\" \\] \\]\n12: event_count         1\n13: first_event         \\[ 1, 0, 1 \\]\n14: last_event          \\[ 1, 0, 1 \\]\n15: parents             \\[ \"out\\.root\" \\]\n-------------------------------\n$"
  )

cet_test(SAM_metadataFromInput_conflictingFields HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c empty.fcl
  --process-name "ShouldFail"
  --sam-inherit-file-type
  --sam-run-type "Canter"
  --sam-data-tier "Perky"
  -s ../SAM_metadataFromInput_01.d/out.root
  -s ../SAM_metadataFromInput_02.d/out.root
  -o out.root
  DATAFILES
  fcl/empty.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "  The value for 'file_type' for the current file is: \"Pescadero\", which conflicts with the value from the first input file \\(\"\"MC\"\"\\)."
  )

set_tests_properties(SAM_metadataFromInput_conflictingFields
  PROPERTIES DEPENDS "SAM_metadataFromInput_01;SAM_metadataFromInput_02")

cet_test(GeneratorSource_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "GeneratorSource_t.fcl"
  DATAFILES
  fcl/GeneratorSource_t.fcl
  )

cet_test(FlushingGenerator_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "FlushingGenerator_t.fcl"
  DATAFILES
  fcl/FlushingGenerator_t.fcl
  )

cet_test(BlockingPrescaler_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config BlockingPrescaler_t.fcl --rethrow-all
  REF "${CMAKE_CURRENT_SOURCE_DIR}/BlockingPrescaler_t-ref.txt"
  DATAFILES
  fcl/BlockingPrescaler_t.fcl
  )

cet_test(test_compressed_simple_01_t.sh PREBUILT
  DATAFILES
  fcl/test_compressed_simple_01.fcl
  fcl/test_compressed_simple_01r.fcl
  fcl/messageDefaults.fcl
)

# Test signal handling.

cet_script(signal_handling_t.sh NO_INSTALL)

cet_test(signal_handling_01_t HANDBUILT
  TEST_EXEC signal_handling_t.sh
  TEST_ARGS -c signal_sigint.fcl SIGINT
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Art has handled signal 2\\."
  DATAFILES
  fcl/signal_sigint.fcl
  )

cet_test(signal_handling_02_t HANDBUILT
  TEST_EXEC signal_handling_t.sh
  TEST_ARGS -c signal_sigint.fcl SIGQUIT
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Art has handled signal 3\\."
  DATAFILES
  fcl/signal_sigint.fcl
  )

cet_test(signal_handling_03_t HANDBUILT
  TEST_EXEC signal_handling_t.sh
  TEST_ARGS -c signal_sigint.fcl SIGTERM
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Art has handled signal 15\\."
  DATAFILES
  fcl/signal_sigint.fcl
  )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  set(sigusr_two_signal 31)
else()
  set(sigusr_two_signal 12)
endif()

cet_test(signal_handling_04_t HANDBUILT
  TEST_EXEC signal_handling_t.sh
  TEST_ARGS -c signal_sigint.fcl SIGUSR2
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "Art has handled signal ${sigusr_two_signal}\\."
  DATAFILES
  fcl/signal_sigint.fcl
  )

foreach (num 1 2)
  cet_test(InFlightConfiguration_0${num}_t HANDBUILT
    TEST_EXEC art_ut
    TEST_ARGS -- --config InFlightConfiguration_0${num}.fcl
    DATAFILES
    fcl/InFlightConfiguration_0${num}.fcl
    )
endforeach()

cet_test(TH1Data_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c TH1Data_t.fcl
  DATAFILES
  fcl/TH1Data_t.fcl
  )

cet_test(PostCloseFileRename_Integration HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c PostCloseFileRename_t.fcl
  DATAFILES
  fcl/PostCloseFileRename_t.fcl
)

cet_test(TestAnalyzerSelect HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "TestAnalyzerSelect.fcl"
  DATAFILES
  fcl/TestAnalyzerSelect.fcl
)

# Issue 5157
cet_test(UnusedModule_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FilterIgnore_t.fcl
  DATAFILES
  fcl/FilterIgnore_t.fcl
  )

SET_TESTS_PROPERTIES(UnusedModule_t PROPERTIES
  FAIL_REGULAR_EXPRESSION
  "not assigned to any path"
  )

# Issue 5114
cet_test(MissingModuleType_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c MissingModuleType_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "ERROR: Configuration of module with label BadModuleConfig encountered the following error:\n"
  DATAFILES
  fcl/MissingModuleType_t.fcl
)

# Issue 5424
cet_test(rename-histfile_t_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c rename-histfile_t.fcl
  DATAFILES
  fcl/rename-histfile_t.fcl
)

cet_test(rename-histfile_t_check HANDBUILT
  TEST_EXEC ls
  TEST_ARGS -l ../rename-histfile_t_w.d/test_1_0_2_8.root
  TEST_PROPERTIES DEPENDS rename-histfile_t_w
)

simple_plugin(TestMetadata "plugin" NO_INSTALL art_Framework_Core fhiclcpp)
cet_test(TestMetadata_plugin_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c TestMetadata_Plugin_t.fcl
  DATAFILES
  fcl/TestMetadata_Plugin_t.fcl
)

cet_test(TestMetadata_verify_hr HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -H -s ../TestMetadata_plugin_t.d/out.root
  TEST_PROPERTIES DEPENDS TestMetadata_plugin_t
  PASS_REGULAR_EXPRESSION
  "^\nFile catalog metadata from file \\.\\./TestMetadata_plugin_t\\.d/out\\.root:\n\n 1: file_type           \"unknown\"\n 2: process_name        \"DEVEL\"\n 3: file_format         \"artroot\"\n 4: file_format_era     \"ART_2011a\"\n 5: file_format_version 8\n 6: start_time          \"${date_regex}\"\n 7: end_time            \"${date_regex}\"\n 8: event_count         1\n 9: first_event         \\[ 1, 0, 1 \\]\n10: last_event          \\[ 1, 0, 1 \\]\n11: key1                \"value1\"\n12: key2                \"value2\"\n-------------------------------\n$"
  )

cet_test(TestMetadata_verify_JSON HANDBUILT
  TEST_EXEC sam_metadata_dumper
  TEST_ARGS -s ../TestMetadata_plugin_t.d/out.root
  TEST_PROPERTIES DEPENDS TestMetadata_plugin_t
  PASS_REGULAR_EXPRESSION
  "^{\n  \"\\.\\./TestMetadata_plugin_t\\.d/out\\.root\": {\n    \"file_type\": \"unknown\",\n    \"process_name\": \"DEVEL\",\n    \"file_format\": \"artroot\",\n    \"file_format_era\": \"ART_2011a\",\n    \"file_format_version\": 8,\n    \"start_time\": \"${date_regex}\",\n    \"end_time\": \"${date_regex}\",\n    \"event_count\": 1,\n    \"first_event\": \\[ 1, 0, 1 \\],\n    \"last_event\": \\[ 1, 0, 1 \\],\n    \"key1\": \"value1\",\n    \"key2\": \"value2\"\n  }\n}\n$"
  )

set_source_files_properties(TestEmptyEventTimestampNoRSRTS_plugin.cc
  PROPERTIES COMPILE_DEFINITIONS TEST_USE_LAST_EVENT_TIMESTAMP
)

simple_plugin(TestEmptyEventTimestamp "plugin" NO_INSTALL art_Framework_Core fhiclcpp)
simple_plugin(TestEmptyEventTimestampNoRSRTS "plugin" NO_INSTALL art_Framework_Core fhiclcpp)

cet_test(TestEmptyEventTimestamp_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c TestEmptyEventTimestamp_t.fcl
  DATAFILES
  fcl/TestEmptyEventTimestamp_t.fcl
  )

cet_test(TestEmptyEventTimestampNoRSRTS_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c TestEmptyEventTimestampNoRSRTS_t.fcl
  DATAFILES
  fcl/TestEmptyEventTimestamp_t.fcl
  fcl/TestEmptyEventTimestampNoRSRTS_t.fcl
  )

simple_plugin(FindManySpeedTestProducer "module" NO_INSTALL)
simple_plugin(FindManySpeedTestAnalyzer "module" NO_INSTALL)

add_subdirectory(testPtrVector)
add_subdirectory(test_tiered_input_01)
add_subdirectory(test_tiered_input_02)
add_subdirectory(test_tiered_input_03)
add_subdirectory(test_tiered_input_04)
add_subdirectory(test_tiered_input_05)

add_subdirectory(event-processor)
add_subdirectory(event-shape)
add_subdirectory(high-memory)
add_subdirectory(output-file-handling)
add_subdirectory(run-subrun-shape)

cet_test(FastCloningInitMessage_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FastCloningInitMessage_01.fcl
  DATAFILES
  fcl/FastCloningInitMessage_01.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\nInitial fast cloning configuration \\(from default\\): true\n"
)

cet_test(FastCloningInitMessage_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FastCloningInitMessage_02.fcl
  DATAFILES
  fcl/FastCloningInitMessage_01.fcl
  fcl/FastCloningInitMessage_02.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\nInitial fast cloning configuration \\(user-set\\): true\n"
)

cet_test(FastCloningInitMessage_03_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FastCloningInitMessage_03.fcl
  DATAFILES
  fcl/FastCloningInitMessage_01.fcl
  fcl/FastCloningInitMessage_03.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\nInitial fast cloning configuration \\(user-set\\): false\n"
)

cet_test(NonexistentPathCheck_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c NonexistentPathCheck_01.fcl
  DATAFILES fcl/NonexistentPathCheck_01.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\n  ERROR: Unknown path x1 specified by user in trigger_paths.\n"
)

cet_test(NonexistentPathCheck_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c NonexistentPathCheck_02.fcl
  DATAFILES fcl/NonexistentPathCheck_02.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\n  ERROR: Unknown path d6 specified by user in end_paths.\n  ERROR: Unknown path x1 specified by user in end_paths.\n"
)

include(legacy-tests)

simple_plugin(RPTest "plugin" NO_INSTALL art_Framework_Core)
simple_plugin(RPTestReader "plugin" NO_INSTALL art_Framework_Core)

cet_test(RPTest_conf_err_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_conf_err_01.fcl
  DATAFILES fcl/RPTest_conf_err_01.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "o1\\.results\\.rpath\\[2\\] \\(rpx\\) is not defined in o1\\.results\\.producers\\.\n"
)

cet_test(RPTest_conf_err_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_conf_err_02.fcl
  DATAFILES fcl/RPTest_conf_err_02.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "o1\\.results\\.rpath2\\[0\\] \\(rpr\\) is a duplicate: previously used in path rpath\\.\n"
)

cet_test(RPTest_conf_err_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_conf_err_03.fcl
  DATAFILES fcl/RPTest_conf_err_03.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "Parameter set o1\\.results is non-empty, but does not contain\n"
)

cet_test(RPTest_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_w.fcl
  DATAFILES fcl/RPTest_w.fcl
)

cet_test(RPTest_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_w2.fcl
  DATAFILES fcl/RPTest_w2.fcl fcl/RPTest_w.fcl
)

cet_test(RPTest_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_r.fcl
  DATAFILES fcl/RPTest_r.fcl
)

cet_test(RPTest_r2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c RPTest_r2.fcl
  DATAFILES fcl/RPTest_r2.fcl
)

set_tests_properties(RPTest_r PROPERTIES DEPENDS "RPTest_w;RPTest_w2")
set_tests_properties(RPTest_r2 PROPERTIES DEPENDS "RPTest_w;RPTest_w2")

simple_plugin(EmptyPSetRegistryChecker "module" NO_INSTALL USE_BOOST_UNIT)

cet_test(EmptyPSetRegistryChecker_w1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c EmptyPSetRegistryChecker_w1.fcl
  DATAFILES fcl/EmptyPSetRegistryChecker_w1.fcl
)

cet_test(EmptyPSetRegistryChecker_r1 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c EmptyPSetRegistryChecker_r1.fcl
  DATAFILES fcl/EmptyPSetRegistryChecker_r1.fcl
  TEST_PROPERTIES DEPENDS EmptyPSetRegistryChecker_w1
)

cet_test(EmptyPSetRegistryChecker_w2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c EmptyPSetRegistryChecker_w2.fcl
  DATAFILES fcl/EmptyPSetRegistryChecker_w1.fcl
  fcl/EmptyPSetRegistryChecker_w2.fcl
)

cet_test(EmptyPSetRegistryChecker_r2 HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c EmptyPSetRegistryChecker_r2.fcl
  DATAFILES fcl/EmptyPSetRegistryChecker_r1.fcl
  fcl/EmptyPSetRegistryChecker_r2.fcl
  TEST_PROPERTIES DEPENDS EmptyPSetRegistryChecker_w2
)
