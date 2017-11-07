set( cpp_files TestIntegration.cpp )

link_libraries(  FWCoreModules  )

add_executable( TestIntegration TestIntegration.cpp )

add_test( NAME TestIntegration COMMAND TestIntegration)

