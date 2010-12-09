# project name
project(FWCoreIntegrationTest)

set( cpp_files TestIntegration.cpp
	       testRunner.cpp )
set( TestThingProducer_sources ThingProducer.cc ThingAlgorithm.cc )
set( TestHistProducer_sources ThingProducer.cc ThingAlgorithm.cc )

set( exe_list TestIntegrationEventHistory
	      TestIntegrationRef
	      TestIntegrationTransRef
	      TestIntegrationGeneratedSource
	      TestIntegrationExternalSource
	      TestIntegrationRawSource
	      TestIntegrationEventSetup
	      TestIntegrationHierarchyExample
	      TestIntegrationServiceExample
	      TestIntegrationView
	      TestIntegrationSeriesOfProcesses
	      TestIntegrationUnscheduledFailOnOutput
	      TestIntegrationParentless
	      TestIntegrationRunMerge
	      TestIntegrationParameterSet
	      CatchStdExceptiontest
	      CatchCmsExceptiontest )

link_libraries( artFrameworkCore
                artFrameworkPluginManager
                art_Utilities
                ${Boost_FILESYSTEM_LIBRARY}  )

# more libraries to be added
add_library(TestThingProducer SHARED ${TestThingProducer_sources} )
add_library(TestHistProducer SHARED ${TestHistProducer_sources} )

foreach(ename ${exe_list} )
  add_executable( ${ename} TestIntegration.cpp  )
  add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
endforeach(file ${cpp_files} )

add_executable( TestFWCoreIntegrationStandalone testRunner.cpp standalone_t.cppunit.cc )
add_test( TestFWCoreIntegrationStandalone ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreIntegrationStandalone )
