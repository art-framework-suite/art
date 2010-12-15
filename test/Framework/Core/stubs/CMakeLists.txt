# project name
project(FWCoreFrameworkStubs)

# version number
set (FWCoreFrameworkStubs_VERSION_MAJOR 1)
set (FWCoreFrameworkStubs_VERSION_MINOR 0)

set( FWCoreFrameworkStubs_sources TestBeginEndJobAnalyzer.cc
				  TestBeginEndJobAnalyzerModule.cc
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

simple_plugin(AddIntsProducer)
simple_plugin(DummySourceModule)
simple_plugin(HistoryAnalyzer)
simple_plugin(IntProducer)
simple_plugin(IntTestAnalzer)
simple_plugin(IntVectorProducer)
simple_plugin(NonProducer)
simple_plugin(SewerModule)
simple_plugin(TestFilterModule)
simple_plugin(TestOutputModule)
simple_plugin(TestResultAnalyzer)
simple_plugin(ToyDoubleProducer)

install ( TARGETS FWCoreFrameworkStubs DESTINATION ${flavorqual_dir}/lib )
