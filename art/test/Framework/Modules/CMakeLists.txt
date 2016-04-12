# project name
project(FWCoreModulesTest)

set( cpp_files TestIntegration.cpp )

link_libraries(  FWCoreModules  )

add_executable( TestIntegration TestIntegration.cpp )

add_test( TestIntegration ${EXECUTABLE_OUTPUT_PATH}/TestIntegration)

