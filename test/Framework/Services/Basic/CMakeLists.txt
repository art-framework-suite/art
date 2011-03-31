# project name
project(FWCoreServicesTest)

set( cpp_files TestFWCoreServicesDriver.cpp )

set( FWCoreServicesTest_sources TestInitRootHandlers.cc
				UnitTestClient.cc )

link_libraries( FWCoreServices )

add_library(FWCoreServicesTest SHARED ${FWCoreServicesTest_sources} )

simple_plugin(RNGS_analyzer  "module" NOINSTALL )
simple_plugin(RNGS_producer  "module" NOINSTALL )
simple_plugin(TestTFileServiceAnalyzer  "module" NOINSTALL )

add_executable( TestFWCoreServicesDriver TestFWCoreServicesDriver.cpp )
target_link_libraries(TestFWCoreServicesDriver FWCoreServices
                			       FrameworkCore
                			       FWCoreParameterSet
                			       FWCorePluginManager
                			       FWCoreUtilities
                			       ${Boost_FILESYSTEM_LIBRARY}
                			       ${Boost_PROGRAM_OPTIONS_LIBRARY}
                			       ${SIGC}
                			       ${CORE}
                			       ${REFLEX}
                			       ${CINTEX} )

install ( TARGETS FWCoreServicesTest DESTINATION ${flavorqual_dir}/lib )

add_test( FWCoreServicesDriver ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreServicesDriver)
