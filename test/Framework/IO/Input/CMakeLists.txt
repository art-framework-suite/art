project(IOPoolInputTest)

set( cpp_files TestPoolInput.cpp )

link_libraries( IOPoolInput
                IOPoolCommon
                DataFormatsCommon
		DataFormatsProvenance
		FWCoreSources
		FWCoreCatalog
		FWCoreServiceRegistry
		FWCoreParameterSet
		FWCoreMessageLogger
		FWCoreFramework
                FWCoreServices
                FWCoreUtilities
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_THREAD_LIBRARY}
		${CLHEP}
                ${CORE} 
		${REFLEX} 
		${CINTEX} 
		${CINT} 
		${RIO} 
		${NET} 
		${TREE} 
		${CPPUNIT} 
		-luuid -ldl )

add_executable( TestPoolInput TestPoolInput.cpp )
add_test( TestPoolInput ${EXECUTABLE_OUTPUT_PATH}/TestPoolInput )
