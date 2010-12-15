# project name
project(FWCoreFrameworkTest)


set( FWCoreFrameworkTestDummyForEventSetup_sources DummyData.cc
                                                   DummyRecord.cc
						   DepRecord.cc
						   Dummy2Record.cc
						   DepOn2Record.cc )
set( FWCoreFrameworkTest_sources MockEventProcessor.cc )
set( FWCoreFrameworkTestDummySource_sources DummySource.cc
                                            stubs/DummySourceModule.cc )
set( TestOutputModule_sources stubs/TestOutputModule.cc )
set( TestFilterModule_sources stubs/TestFilterModule.cc )
set( TestTriggerNames_sources stubs/TestTriggerNames.cc )
set( TestMergeResults_sources stubs/TestMergeResults.cc )
set( TestMod_sources stubs/TestMod.cc )
set( TestToyModules_sources stubs/ToyModules.cc )
set( TestSchedulerModule1_sources stubs/TestSchedulerModule1.cc )
set( TestSchedulerModule2_sources stubs/TestSchedulerModule2.cc )
set( TestLoadableDummyFinder_sources stubs/LoadableDummyFinder.cc )
set( TestLoadableDummyESSource_sources stubs/LoadableDummyESSource.cc )
set( TestLoadableDummyEventSetupRecordRetriever_sources stubs/LoadableDummyEventSetupRecordRetriever.cc )
set( TestLoadableDummyProvider_sources stubs/LoadableDummyProvider.cc )
set( FWCoreFrameworkTestDummyLooper_sources stubs/DummyLooper.cc )
set( FWCoreFrameworkTestESDummyDataAnalyzer_sources stubs/TestESDummyDataAnalyzer.cc )
set( FWCoreFrameworkTestPRegisterModules_sources stubs/TestPRegisterModule2.cc
                                                 stubs/TestPRegisterModule1.cc
						 stubs/TestPRegisterModules.cc )
set( FWCoreFrameworkTestBeginEndJobAnalyzer_sources stubs/TestBeginEndJobAnalyzer.cc
                                                    stubs/TestBeginEndJobAnalyzerModule.cc )
set( FWCoreFrameworkTestFailuresAnalyzer_sources stubs/TestFailuresAnalyzer.cc )
set( FWCoreFrameworkHistoryAnalyzer_sources stubs/HistoryAnalyzer.cc )


set( TestFWCoreFramework_sources testRunner.cpp
                                 DummySource.cc
				 edproducer_productregistry_callback.cc
				 generichandle_t.cppunit.cc
				 interval_t.cppunit.cc
				 iovsyncvalue_t.cppunit.cc
				 maker2_t.cppunit.cc
				 maker_t.cppunit.cc
#				 productregistry.cppunit.cc
)
#set( FWCoreFrameworkTestDummyService_sources stubs/DependsOnDummyService.cc
#                                             stubs/DummyService.cc
#                                             stubs/DummyServiceE0.cc )
#set( FWCoreFrameworkTestDummyServicePlugin_sources stubs/Modules.cc )
set( TestFWCoreFrameworkeventprocessor_sources  eventprocessor_t.cppunit.cc
						eventprocessor2_t.cppunit.cc
						stubs/TestBeginEndJobAnalyzer.cc )

#add_library(FWCoreFrameworkTestDummyService SHARED ${art_Framework_Services_Registry_TestDummyService_sources} )
#add_library(FWCoreFrameworkTestDummyServicePlugin SHARED ${art_Framework_Services_Registry_TestDummyServicePlugin_sources} )

link_libraries( art_Framework_Services_Registry
  art_Framework_PluginManager
  art_Utilities
  art_Framework_Core
  ${Boost_FILESYSTEM_LIBRARY}
  ${SIGC}
	${CORE}
	${REFLEX}
  ${CINTEX}
  ${CINT}
 )


add_executable( TestFWCoreFramework ${TestFWCoreFramework_sources} )
#add_executable( TestFWCoreFrameworkeventprocessor  ${TestFWCoreFrameworkeventprocessor_sources} )
add_executable( TestFWCoreFrameworkeventprincipal  testRunner.cpp eventprincipal_t.cppunit.cc )
add_executable( TestFWCoreFrameworkEvent Event_t.cpp )
# add_executable( TestFWCoreFrameworkView View_t.cpp )
add_executable( TestFWCoreFrameworkCmsRun TestDriver.cpp )
add_executable( TestFWCoreFrameworkESRefer TestDriver.cpp )
add_executable( TestFWCoreFrameworkUnscheduled TestDriver.cpp )
add_executable( TestFWCoreFrameworkReplace TestDriver.cpp )
add_executable( TestFWCoreFrameworkESProducerLooper TestDriver.cpp )
add_executable( TestFWCoreFrameworkTrigBit TestDriver.cpp )
add_executable( TestFWCoreFrameworkTrigMask TestDriver.cpp )
add_executable( TestXMLSafeException TestDriver.cpp )
add_executable( TestFWCoreFrameworkGroupSelector GroupSelector_t.cpp )
add_executable( TestFWCoreFrameworkEventSelector EventSelector_t.cpp )
add_executable( TestFWCoreFrameworkEventSelWildcard EventSelWildcard_t.cpp )
add_executable( TestFWCoreFrameworkEventSelExc EventSelExc_t.cpp )
add_executable( TestFWCoreFrameworkEventSelOverlap EventSelOverlap_t.cpp )
add_executable( TestFWCoreFrameworkCurrentProcessingContext CurrentProcessingContext_t.cpp )
add_executable( TestFWCoreFrameworkCPCSentry CPCSentry_t.cpp )
add_executable( TestFWCoreFrameworkRegistryTemplate RegistryTemplate_t.cpp )
add_executable( TestFWCoreOutputModuleUtilities OutputModuleUtilities_t.cpp )
add_executable( TestFWCoreFrameworkStatemachine statemachine_t.cc MockEventProcessor.cc )
target_link_libraries(TestFWCoreFrameworkStatemachine
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
add_executable( TestFWCoreFrameworkStatemachineSH TestDriver.cpp )

add_test( TestFWCoreFramework ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFramework )
#add_test( TestFWCoreFrameworkeventprocessor  ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkeventprocessor )
add_test( TestFWCoreFrameworkeventprincipal  ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkeventprincipal )
add_test( TestFWCoreFrameworkEvent ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEvent )
#add_test( TestFWCoreFrameworkView ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkView )
add_test( TestFWCoreFrameworkCmsRun ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCmsRun )
add_test( TestFWCoreFrameworkESRefer ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkESRefer )
add_test( TestFWCoreFrameworkUnscheduled ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkUnscheduled )
add_test( TestFWCoreFrameworkReplace ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkReplace )
add_test( TestFWCoreFrameworkESProducerLooper ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkESProducerLooper )
add_test( TestFWCoreFrameworkTrigBit ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkTrigBit )
add_test( TestFWCoreFrameworkTrigMask ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkTrigMask )
add_test( TestXMLSafeException ${EXECUTABLE_OUTPUT_PATH}/TestXMLSafeException )
add_test( TestFWCoreFrameworkGroupSelector ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkGroupSelector )
add_test( TestFWCoreFrameworkEventSelector ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelector )
add_test( TestFWCoreFrameworkEventSelWildcard ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelWildcard )
add_test( TestFWCoreFrameworkEventSelExc ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelExc )
add_test( TestFWCoreFrameworkEventSelOverlap ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkEventSelOverlap )
add_test( TestFWCoreFrameworkCurrentProcessingContext ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCurrentProcessingContext )
add_test( TestFWCoreFrameworkCPCSentry ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkCPCSentry )
add_test( TestFWCoreFrameworkRegistryTemplate ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkRegistryTemplate )
add_test( TestFWCoreOutputModuleUtilities ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreOutputModuleUtilities )
add_test( TestFWCoreFrameworkStatemachine ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkStatemachine )
add_test( TestFWCoreFrameworkStatemachineSH ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreFrameworkStatemachineSH )
