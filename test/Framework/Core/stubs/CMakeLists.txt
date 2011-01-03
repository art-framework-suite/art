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

simple_plugin(AddIntsProducer "plugin")
simple_plugin(DummySourceModule "plugin")
simple_plugin(HistoryAnalyzer "plugin")
simple_plugin(IntProducer "plugin")
simple_plugin(IntTestAnalzer "plugin")
simple_plugin(IntVectorProducer "plugin")
simple_plugin(NonProducer "plugin")
simple_plugin(SewerModule "plugin")
simple_plugin(TestFilterModule "plugin")
simple_plugin(TestOutputModule "plugin")
simple_plugin(TestResultAnalyzer "plugin")
simple_plugin(ToyDoubleProducer "plugin")

install ( TARGETS FWCoreFrameworkStubs DESTINATION ${flavorqual_dir}/lib )
