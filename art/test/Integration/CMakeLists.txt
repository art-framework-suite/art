art_make_library(SOURCE
  ToySource.cc
  LIBRARIES art_Framework_IO_Sources
  art_Framework_Core
  fhiclcpp
  NO_INSTALL)

simple_plugin(AddIntsProducer             "module" NO_INSTALL)
simple_plugin(AssnsAnalyzer               "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(AssnsMultipleProduces       "module" NO_INSTALL)
simple_plugin(AssnsProducer               "module" NO_INSTALL)
simple_plugin(AssnsWrongPut               "module" NO_INSTALL)
simple_plugin(BareStringAnalyzer          "module" NO_INSTALL)
simple_plugin(BareStringProducer          "module" NO_INSTALL)
simple_plugin(CompressedIntProducer       "module" NO_INSTALL)
simple_plugin(CompressedIntTestAnalyzer   "module" NO_INSTALL)
simple_plugin(ConcurrentEngineRetrieval   "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(DeferredIsReadyWithAssnsAnalyzer "module" NO_INSTALL)
simple_plugin(DeferredIsReadyWithAssnsProducer "module" NO_INSTALL)
simple_plugin(DerivedPtrVectorProducer    "module" NO_INSTALL)
simple_plugin(DifferentHistoriesAnalyzer  "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(DoubleProducer              "module" NO_INSTALL)
simple_plugin(DoubleTestAnalyzer          "module" NO_INSTALL)
simple_plugin(DropTestAnalyzer            "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(DropTestParentageFaker      "module" NO_INSTALL)
simple_plugin(EventIDFilterChecker        "module" NO_INSTALL)
simple_plugin(FailingAnalyzer             "module" NO_INSTALL)
simple_plugin(FailingProducer             "module" NO_INSTALL)
simple_plugin(UnputtingProducer           "module" NO_INSTALL)
simple_plugin(FlushingGenerator           "source" NO_INSTALL)
simple_plugin(FlushingGeneratorTest       "module" NO_INSTALL)
simple_plugin(FlushingGeneratorTestFilter "module" NO_INSTALL)
simple_plugin(GeneratorTest               "source" NO_INSTALL)
simple_plugin(ImplicitRSAssignmentAnalyzer "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(ImplicitRSAssignmentProducer "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(IntProducer                 "module" NO_INSTALL)
simple_plugin(IntReaderThenProducer       "module" NO_INSTALL)
simple_plugin(IntTestAnalyzer             "module" NO_INSTALL)
simple_plugin(IntVectorAnalyzer           "module" NO_INSTALL)
simple_plugin(IntVectorProducer           "module" NO_INSTALL)
simple_plugin(IntArrayAnalyzer            "module" NO_INSTALL)
simple_plugin(IntArrayProducer            "module" NO_INSTALL)
simple_plugin(IntsVerifier                "module" NO_INSTALL)
simple_plugin(MessagePrinter              "tool"   NO_INSTALL)
simple_plugin(MockClusterListAnalyzer     "module" NO_INSTALL)
simple_plugin(MockClusterListProducer     "module" NO_INSTALL)
simple_plugin(ProductIDGetter             "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(ProductIDGetterNoPut        "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(ProductIDGetterAnalyzer     "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(PtrListAnalyzer             "module" NO_INSTALL)
simple_plugin(PtrVectorSimpleAnalyzer     "module" NO_INSTALL)
simple_plugin(PtrmvAnalyzer               "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(PtrmvProducer               "module" NO_INSTALL)
simple_plugin(RandomNumberSaveTest        "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(ReadNextArbitrator          "tool"   NO_INSTALL)
simple_plugin(RunSubRunProductProducerNoPut "module" NO_INSTALL)
simple_plugin(SAMMetadataConcurrentTest   "module" NO_INSTALL USE_BOOST_UNIT art_Framework_Services_System_FileCatalogMetadata_service)
simple_plugin(SAMMetadataTest             "module" NO_INSTALL art_Framework_Services_System_FileCatalogMetadata_service)
simple_plugin(ServiceUsing                "service" NO_INSTALL hep_concurrency)
simple_plugin(SigIntAnalyzer              "module" NO_INSTALL)
simple_plugin(SimpleDerivedAnalyzer       "module" NO_INSTALL)
simple_plugin(SimpleDerivedProducer       "module" NO_INSTALL)
simple_plugin(SimpleSource                "source" NO_INSTALL)
simple_plugin(TestAnalyzerSelect          "module" NO_INSTALL)
simple_plugin(TestBitsOutput              "module" NO_INSTALL)
simple_plugin(TestFilter                  "module" NO_INSTALL)
simple_plugin(TestFilterSpecificEvents    "module" NO_INSTALL)
simple_plugin(TestOutput                  "module" NO_INSTALL)
simple_plugin(TestProvenanceDumper        "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(TestResultAnalyzer          "module" NO_INSTALL)
simple_plugin(TestServiceUsingService     "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(TestSimpleMemoryCheckProducer "module" NO_INSTALL)
simple_plugin(TestTimeTrackerProducer     "module" NO_INSTALL)
simple_plugin(TestTimeTrackerFilter       "module" NO_INSTALL)
simple_plugin(TestTimeTrackerAnalyzer     "module" NO_INSTALL)
simple_plugin(Throwing                    "service" NO_INSTALL hep_concurrency)
simple_plugin(ToyProductAnalyzer          "module" NO_INSTALL)
simple_plugin(ToyProductFilter            "module" NO_INSTALL)
simple_plugin(ToyProductFilterReplicated  "module" NO_INSTALL)
simple_plugin(ToyProductFilterAsync       "module" NO_INSTALL)
simple_plugin(ToyProductProducer          "module" NO_INSTALL)
simple_plugin(ToyProductProducerMultiput  "module" NO_INSTALL)
simple_plugin(ToyRawFileInput             "source" NO_INSTALL USE_BOOST_UNIT art_test_Integration)
simple_plugin(ToyRawInput                 "source" NO_INSTALL USE_BOOST_UNIT art_test_Integration)
simple_plugin(ToyRawInputTester           "module" NO_INSTALL)
simple_plugin(ToyRawProductAnalyzer       "module" NO_INSTALL)
simple_plugin(U_S                         "module" NO_INSTALL ALLOW_UNDERSCORES)
simple_plugin(ValidHandleTester           "module" NO_INSTALL)
simple_plugin(Wanted                      "service" NO_INSTALL)
simple_plugin(addInts                     "tool"   NO_INSTALL)

########################################################################
# Old tests converted from CMS FWCore/Framework/test
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

foreach(test ${test_list})
  cet_test(${test} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ${test}.fcl
    DATAFILES
    fcl/${test}.fcl
    fcl/messageDefaults.fcl)
endforeach()
########################################################################


set(NUM_THREADS 1 2 4 8)

cet_test(EventIDFilter_t HANDBUILT
  TEST_EXEC art
  PARG_J -j ${NUM_THREADS}
  TEST_ARGS --rethrow-all --config EventIDFilter_t.fcl -n 10
  DATAFILES fcl/EventIDFilter_t.fcl)

cet_test(GetProductsCrossPaths_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c getProductsCrossPaths_t1.fcl
  DATAFILES fcl/getProductsCrossPaths_t1.fcl)

cet_test(GetProductsCrossPaths_t2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c getProductsCrossPaths_t2.fcl
  DATAFILES
  fcl/getProductsCrossPaths_t1.fcl
  fcl/getProductsCrossPaths_t2.fcl
  TEST_PROPERTIES WILL_FAIL TRUE)

cet_test(GetManyByType_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c getManyByType_t1.fcl
  DATAFILES fcl/getManyByType_t1.fcl)

cet_test(ptr_list_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config ptr_list_01.fcl
  DATAFILES
  fcl/ptr_list_01.fcl
  fcl/messageDefaults.fcl)

cet_test(test_unputtingProducer_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c test_unputtingProducer.fcl
  DATAFILES
  fcl/test_unputtingProducer.fcl
  fcl/messageDefaults.fcl
  TEST_PROPERTIES WILL_FAIL TRUE)

cet_test(issue_0923_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c issue_0923b.fcl --debug-config config.out
  DATAFILES
  fcl/issue_0923a.fcl
  fcl/issue_0923b.fcl)

cet_test(issue_0923_r HANDBUILT
  TEST_EXEC diff
  TEST_ARGS -u ${CMAKE_CURRENT_SOURCE_DIR}/issue_0923_ref.txt ../issue_0923_w.d/config.out
  TEST_PROPERTIES DEPENDS issue_0923_w)

cet_test(issue_0940_t.sh PREBUILT
  DATAFILES
  fcl/issue_0940.fcl
  fcl/messageDefaults.fcl)

cet_test(issue_0930_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c issue_0930.fcl
  DATAFILES
  fcl/issue_0930.fcl
  fcl/messageDefaults.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "LogicError BEGIN")

cet_test(ToyProductProducerFilter_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c toyProductProducerFilter_t.fcl
  DATAFILES fcl/toyProductProducerFilter_t.fcl)

cet_test(ToyProductProducerFilterReplicated_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyProductProducerFilterReplicated_t.fcl -j 10
  DATAFILES fcl/ToyProductProducerFilterReplicated_t.fcl)

cet_test(ToyProductProducerFilterAsync_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c ToyProductProducerFilterAsync_t.fcl
  DATAFILES fcl/ToyProductProducerFilterAsync_t.fcl)

cet_test(ToyProductProducerAnalyzer_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c toyProductProducerAnalyzer_t.fcl
  DATAFILES fcl/toyProductProducerAnalyzer_t.fcl)

# The following test throws an exception and does not return status
# code 0.  The test is checking that the correct exception has been
# thrown, as indicated by the regular-expression pattern that matches
# the exception message.
cet_test(SimpleSource_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c SimpleSource_t_01.fcl
  DATAFILES fcl/SimpleSource_t_01.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "readNext returned true but no RunPrincipal has been set")

foreach(num 01 02 03)
  cet_test(ToyProductProducerMultiput_t_${num} HANDBUILT
    TEST_EXEC art
    TEST_ARGS --rethrow-all -c ToyProductProducerMultiput_t_${num}.fcl
    DATAFILES fcl/ToyProductProducerMultiput_t_${num}.fcl
    TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION "Attempt to put multiple products with the"
    PASS_REGULAR_EXPRESSION "following description onto the")
endforeach()

cet_test(ProductIDGetter_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetter_t.fcl"
  DATAFILES fcl/ProductIDGetter_t.fcl)

cet_test(ProductIDGetterNoPut_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t.fcl"
  DATAFILES fcl/ProductIDGetterNoPut_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'")

cet_test(ProductIDGetterNoPut_t_01a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t_01.fcl"
  DATAFILES fcl/ProductIDGetterNoPut_t_01.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'")

cet_test(ProductIDGetterNoPut_t_01b HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all --errorOnFailureToPut=false -c "ProductIDGetterNoPut_t_01.fcl"
  DATAFILES fcl/ProductIDGetterNoPut_t_01.fcl)

cet_test(ProductIDGetterNoPut_t_02a HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "ProductIDGetterNoPut_t_02.fcl"
  DATAFILES fcl/ProductIDGetterNoPut_t_02.fcl)

cet_test(ProductIDGetterNoPut_t_02b HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all --errorOnFailureToPut=true -c "ProductIDGetterNoPut_t_02.fcl"
  DATAFILES fcl/ProductIDGetterNoPut_t_02.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The following products have been declared with 'produces'")

cet_test(DeferredIsReadyWithAssns_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all --config "DeferredIsReadyWithAssns_t.fcl"
  DATAFILES fcl/DeferredIsReadyWithAssns_t.fcl)

# Write a data file containing stored products and stored random number
# states for subsequent tests.
cet_test(RNGConcurrentAccess_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "RNGConcurrentAccess_t.fcl"
  DATAFILES fcl/RNGConcurrentAccess_t.fcl)

cet_test(AssnsWrongPut_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "AssnsWrongPut_t.fcl"
  DATAFILES fcl/AssnsWrongPut_t.fcl
  TEST_PROPERTIES
  # std::size_t may have different representations on different
  # platforms.  Therefore we just require that it be unsigned, and
  # wildcard ('.*') the rest of it.
  PASS_REGULAR_EXPRESSION "No product is registered for.*art::Assns<std::string,unsigned.*,void>")

cet_test(AssnsMultipleProduces_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "AssnsMultipleProduces_t.fcl"
  DATAFILES fcl/AssnsMultipleProduces_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "That friendly name has already been registered for this module.")

cet_test(ServiceUsingService_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c "ServiceUsingService_t.fcl"
  DATAFILES fcl/ServiceUsingService_t.fcl)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  cet_test(MemoryTracker_t HANDBUILT
    TEST_EXEC art
    PARG_J -j ${NUM_THREADS}
    TEST_ARGS -c memoryTracker.fcl
    DATAFILES
    fcl/memoryTracker.fcl
    fcl/messageDefaults.fcl
    TEST_PROPERTIES
    PASS_REGULAR_EXPRESSION "Details saved in.*passed = 10.*status 0\\.")
endif()

cet_test(TimeTracker_t1 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c TimeTracker_t1.fcl
  DATAFILES fcl/TimeTracker_t1.fcl
  REF "${CMAKE_CURRENT_SOURCE_DIR}/TimeTracker_t1-ref.txt"
  OUTPUT_FILTERS "filter-timeTracker-output" "DEFAULT")

# Exercise TimeTracker's parallel execution handling
cet_test(TimeTracker_t2 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c TimeTracker_t2.fcl -j10
  DATAFILES
  fcl/TimeTracker_t2.fcl
  fcl/TimeTracker_t1.fcl)

cet_test(SAM_metadata_concurrent_insertion HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --rethrow-all -c "SAMMetadata_concurrent_insertion_t.fcl"
  DATAFILES fcl/SAMMetadata_concurrent_insertion_t.fcl)

cet_test(GeneratorSource_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "GeneratorSource_t.fcl"
  DATAFILES fcl/GeneratorSource_t.fcl)

cet_test(FlushingGenerator_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "FlushingGenerator_t.fcl"
  DATAFILES fcl/FlushingGenerator_t.fcl)

cet_test(BlockingPrescaler_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --config BlockingPrescaler_t.fcl --rethrow-all
  REF "${CMAKE_CURRENT_SOURCE_DIR}/BlockingPrescaler_t-ref.txt"
  DATAFILES fcl/BlockingPrescaler_t.fcl)

# Test signal handling: specifically, check that whenever a signal is
# received, the art framework appropriately backs out of the event
# processing level.  We set errorOnSIGINT to false so that we need
# only one STDOUT reference file (i.e. return code is 0).
foreach(SIGNAL sigint sigquit sigterm sigusr2)
  # The SIGUSR2 return code differs between Linux and Darwin, so we
  # must have separate references.
  if (${SIGNAL} MATCHES "sigusr2")
    set(SIGNAL_PLATFORM "sigusr2_${CMAKE_SYSTEM_NAME}")
  else ()
    set(SIGNAL_PLATFORM ${SIGNAL})
  endif()
  cet_test(signal_handling_${SIGNAL}_t HANDBUILT
    TEST_EXEC art
    TEST_ARGS -c signal_${SIGNAL}.fcl --errorOnSIGINT=false --process-name="Test${SIGNAL}"
    REF "${CMAKE_CURRENT_SOURCE_DIR}/signal_handling_t-ref.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/signal_handling_${SIGNAL_PLATFORM}_rc-ref.txt"
    DATAFILES
    fcl/signal_base.fcl
    fcl/signal_${SIGNAL}.fcl)
endforeach()

cet_test(ImplicitRSAssignment_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- --config implicitRSAssignment_t.fcl
  DATAFILES fcl/implicitRSAssignment_t.fcl)

cet_test(TestAnalyzerSelect HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c "TestAnalyzerSelect.fcl"
  DATAFILES fcl/TestAnalyzerSelect.fcl)

cet_test(UnusedModule_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c FilterIgnore_t.fcl
  DATAFILES fcl/FilterIgnore_t.fcl)

SET_TESTS_PROPERTIES(UnusedModule_t PROPERTIES
  FAIL_REGULAR_EXPRESSION "not assigned to any path")

cet_test(MissingModuleType_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c MissingModuleType_t.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION
  "ERROR: Configuration of module with label BadModuleConfig encountered the following error:\n"
  DATAFILES fcl/MissingModuleType_t.fcl)

set_source_files_properties(TestEmptyEventTimestampNoRSRTS_plugin.cc
  PROPERTIES COMPILE_DEFINITIONS TEST_USE_LAST_EVENT_TIMESTAMP)

simple_plugin(TestEmptyEventTimestamp "plugin" NO_INSTALL art_Framework_Core fhiclcpp)
simple_plugin(TestEmptyEventTimestampNoRSRTS "plugin" NO_INSTALL art_Framework_Core fhiclcpp)

cet_test(TestEmptyEventTimestamp_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c TestEmptyEventTimestamp_t.fcl
  DATAFILES fcl/TestEmptyEventTimestamp_t.fcl)

cet_test(TestEmptyEventTimestampNoRSRTS_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c TestEmptyEventTimestampNoRSRTS_t.fcl
  DATAFILES
  fcl/TestEmptyEventTimestamp_t.fcl
  fcl/TestEmptyEventTimestampNoRSRTS_t.fcl)

add_subdirectory(assns-iter)
add_subdirectory(event-processor)

cet_test(NonexistentPathCheck_01_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c NonexistentPathCheck_01.fcl
  DATAFILES fcl/NonexistentPathCheck_01.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\n  ERROR: Unknown path x1 specified by user in trigger_paths.\n")

cet_test(NonexistentPathCheck_02_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c NonexistentPathCheck_02.fcl
  DATAFILES fcl/NonexistentPathCheck_02.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "\n  ERROR: Unknown path x1 specified by user in end_paths.\n  ERROR: Unknown path d6 specified by user in end_paths.\n")

simple_plugin(SimpleServiceTest "service" NO_INSTALL)
simple_plugin(SimpleServiceTestAnalyzer "module" NO_INSTALL art_test_Integration_SimpleServiceTest_service)

cet_test(SimpleServiceTest_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c SimpleServiceTest_t.fcl
  DATAFILES fcl/SimpleServiceTest_t.fcl)

cet_test(PruneConfig_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c prune_config_t_01.fcl --prune-config
  DATAFILES fcl/prune_config_t_01.fcl
  TEST_PROPERTIES WILL_FAIL TRUE)
