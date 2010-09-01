# project name
project(FWCorePluginManagerTest)

set( cpp_files FileOperations_t.cpp )

link_libraries( FWCorePluginManager
		${Boost_FILESYSTEM_LIBRARY}  )

add_library(TestFWCorePluginManagerDummyFactory SHARED DummyFactory.cc )
add_library(TestFWCorePluginManagerDummyPlugins SHARED OneDummy.cc TwoDummy.cc )

add_executable( TestFWCorePluginManagerPluginFactoryManager pluginfactorymanager_t.cc )
add_executable( TestFWCorePluginManagerPluginManager pluginmanager_t.cc )
add_executable( TestFWCorePluginManagerCacheParser cacheparser_t.cc )
add_executable( TestFWCorePluginManagerPluginFactory pluginfactory_t.cc )
add_executable( FileOperations_t FileOperations_t.cpp )
target_link_libraries( TestFWCorePluginManagerPluginFactory TestFWCorePluginManagerDummyFactory )
target_link_libraries( TestFWCorePluginManagerPluginManager TestFWCorePluginManagerDummyPlugins
                                                            TestFWCorePluginManagerDummyFactory )

add_test(FileOperations                          ${EXECUTABLE_OUTPUT_PATH}/FileOperations_t  )
add_test(FWCorePluginManagerPluginFactoryManager ${EXECUTABLE_OUTPUT_PATH}/TestFWCorePluginManagerPluginFactoryManager  )
add_test(FWCorePluginManagerPluginManager        ${EXECUTABLE_OUTPUT_PATH}/TestFWCorePluginManagerPluginManager )
add_test(FWCorePluginManagerCacheParser          ${EXECUTABLE_OUTPUT_PATH}/TestFWCorePluginManagerCacheParser )
add_test(FWCorePluginManagerPluginFactory        ${EXECUTABLE_OUTPUT_PATH}/TestFWCorePluginManagerPluginFactory )
