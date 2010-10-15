# project name
project(FWCoreFrameworkStubs)

# version number
set (FWCoreFrameworkStubs_VERSION_MAJOR 1)
set (FWCoreFrameworkStubs_VERSION_MINOR 0)

set( FWCoreFrameworkStubs_sources LoadableDummyESSource.cc
				  LoadableDummyEventSetupRecordRetriever.cc
				  LoadableDummyFinder.cc
				  LoadableDummyProvider.cc
				  TestBeginEndJobAnalyzer.cc
				  TestBeginEndJobAnalyzerModule.cc
				  TestESDummyDataAnalyzer.cc
				  TestFailuresAnalyzer.cc
				  TestFilterModule.cc
				  TestMergeResults.cc
				  TestMod.cc
				  TestPRegisterModule1.cc
				  TestPRegisterModule2.cc
				  TestPRegisterModules.cc
				  TestSchedulerModule1.cc
				  TestSchedulerModule2.cc
				  TestTriggerNames.cc
				  ToyModules.cc )

link_libraries( FWCoreFramework )

add_library(FWCoreFrameworkStubs SHARED ${FWCoreFrameworkStubs_sources} )

simple_plugin(AddIntsProducer_plugin)
simple_plugin(DummyLooper_plugin)
simple_plugin(DummySourceModule_plugin)
simple_plugin(HistoryAnalyzer_plugin)
simple_plugin(IntProducer_plugin)
simple_plugin(IntTestAnalzer_plugin)
simple_plugin(IntVectorProducer_plugin)
simple_plugin(NonProducer_plugin)
simple_plugin(SewerModule_plugin)
simple_plugin(TestFilterModule_plugin)
simple_plugin(TestOutputModule_plugin)
simple_plugin(TestResultAnalyzer_plugin)
simple_plugin(ToyDoubleProducer_plugin)

install ( TARGETS FWCoreFrameworkStubs DESTINATION ${flavorqual_dir}/lib )
