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

simple_plugin(AddIntsProducer  "module" NO_INSTALL )
simple_plugin(HistoryAnalyzer  "module" NO_INSTALL )
simple_plugin(IntProducer  "module" NO_INSTALL )
simple_plugin(IntTestAnalzer  "module" NO_INSTALL )
simple_plugin(IntVectorProducer  "module" NO_INSTALL )
simple_plugin(NonProducer  "module" NO_INSTALL )
simple_plugin(SewerModule  "module" NO_INSTALL )
simple_plugin(TestFilterModule  "module" NO_INSTALL )
simple_plugin(TestOutputModule  "module" NO_INSTALL )
simple_plugin(TestResultAnalyzer  "module" NO_INSTALL )
simple_plugin(ToyDoubleProducer  "module" NO_INSTALL )

install ( TARGETS FrameworkCoreStubs DESTINATION ${flavorqual_dir}/lib )
