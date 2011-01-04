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

simple_plugin(AddIntsProducer  "module")
simple_plugin(DummySourceModule  "source")
simple_plugin(HistoryAnalyzer  "module")
simple_plugin(IntProducer  "module")
simple_plugin(IntTestAnalzer  "module")
simple_plugin(IntVectorProducer  "module")
simple_plugin(NonProducer  "module")
simple_plugin(SewerModule  "module")
simple_plugin(TestFilterModule  "module")
simple_plugin(TestOutputModule  "module")
simple_plugin(TestResultAnalyzer  "module")
simple_plugin(ToyDoubleProducer  "module")

install ( TARGETS FWCoreFrameworkStubs DESTINATION ${flavorqual_dir}/lib )
