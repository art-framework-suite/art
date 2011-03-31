# project name
project(FrameworkCoreStubs)

# version number
set (FrameworkCoreStubs_VERSION_MAJOR 1)
set (FrameworkCoreStubs_VERSION_MINOR 0)

set( FrameworkCoreStubs_sources TestBeginEndJobAnalyzer.cc
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

link_libraries( FrameworkCore )

add_library(FrameworkCoreStubs SHARED ${FrameworkCoreStubs_sources} )

simple_plugin(AddIntsProducer  "module" NOINSTALL )
simple_plugin(HistoryAnalyzer  "module" NOINSTALL )
simple_plugin(IntProducer  "module" NOINSTALL )
simple_plugin(IntTestAnalzer  "module" NOINSTALL )
simple_plugin(IntVectorProducer  "module" NOINSTALL )
simple_plugin(NonProducer  "module" NOINSTALL )
simple_plugin(SewerModule  "module" NOINSTALL )
simple_plugin(TestFilterModule  "module" NOINSTALL )
simple_plugin(TestOutputModule  "module" NOINSTALL )
simple_plugin(TestResultAnalyzer  "module" NOINSTALL )
simple_plugin(ToyDoubleProducer  "module" NOINSTALL )

install ( TARGETS FrameworkCoreStubs DESTINATION ${flavorqual_dir}/lib )
