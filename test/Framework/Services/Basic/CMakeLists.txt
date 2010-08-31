# project name
project(FWCoreServicesTest)

set( cpp_files TestFWCoreServicesDriver.cpp )

set( FWCoreServicesTest_sources TestInitRootHandlers.cc
                                TestTFileServiceAnalyzer.cc
				UnitTestClient.cc )

link_libraries( FWCoreServices )

add_library(FWCoreServicesTest SHARED ${FWCoreServicesTest_sources} )

simple_plugin(RNGS_analyzer_plugin)
simple_plugin(RNGS_producer_plugin)
simple_plugin(TestTFileServiceAnalyzer_plugin)

add_executable( TestFWCoreServicesDriver TestFWCoreServicesDriver.cpp )
target_link_libraries(TestFWCoreServicesDriver FWCoreServices
                			       FWCoreFramework
                			       FWCoreParameterSet
                			       FWCorePluginManager
                			       FWCoreUtilities
                			       ${Boost_FILESYSTEM_LIBRARY} 
                			       ${Boost_PROGRAM_OPTIONS_LIBRARY} 
                			       ${SIGC}
                			       ${CORE}
                			       ${REFLEX} 
                			       ${CINTEX} )

install ( TARGETS FWCoreServicesTest DESTINATION lib )

add_test( FWCoreServicesDriver ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreServicesDriver)
