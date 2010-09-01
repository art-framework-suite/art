# project name
project(FWCoreServiceRegistryTest)

set( TestDummyService_sources stubs/DependsOnDummyService.cc
			      stubs/DummyService.cc
			      stubs/DummyServiceE0.cc )

link_libraries( FWCoreServiceRegistryTestDummyService )

add_library(FWCoreServiceRegistryTestDummyService SHARED ${TestDummyService_sources} )
add_library(FWCoreServiceRegistryTestDummyServicePlugin SHARED stubs/Modules.cc )

add_executable( testServiceRegistry serviceregistry_t.cppunit.cpp
                                  servicesmanager_t.cppunit.cc
				  connect_but_block_self_t.cppunit.cc )
add_executable( servicesmanager_order servicesmanager_order.cpp )
target_link_libraries( testServiceRegistry FWCoreServiceRegistryTestDummyService
                			   FWCoreServiceRegistry
                			   FWCoreParameterSet
                			   FWCorePluginManager
                			   FWCoreUtilities
					   DataFormatsProvenance
					   ${MATRIX} )
target_link_libraries( servicesmanager_order FWCoreServiceRegistryTestDummyService
                			     FWCoreServiceRegistry
                			     FWCoreParameterSet
                			     FWCorePluginManager
                			     FWCoreUtilities
					     DataFormatsProvenance
					     ${MATRIX} )

add_test( testServiceRegistry   ${EXECUTABLE_OUTPUT_PATH}/testServiceRegistry)
add_test( servicesmanager_order ${EXECUTABLE_OUTPUT_PATH}/servicesmanager_order)
