project(IOPoolOutputTest)

set( cpp_files TestPoolOutput.cpp )

link_libraries( IOPoolOutput
                IOPoolCommon
                DataFormatsCommon
		DataFormatsProvenance
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

add_executable( TestPoolOutput TestPoolOutput.cpp )
add_test( TestPoolOutput ${EXECUTABLE_OUTPUT_PATH}/TestPoolOutput )
