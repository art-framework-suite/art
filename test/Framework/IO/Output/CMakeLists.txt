project(IORootOutputTest)

set( cpp_files TestRootOutput.cpp )

link_libraries( IORootOutput
                IORootCommon
                DataFormatsCommon
		DataFormatsProvenance
		FWCoreServiceRegistry
		FWCoreParameterSet
		FWCoreMessageLogger
		FrameworkCore
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

add_executable( TestRootOutput TestRootOutput.cpp )
add_test( TestRootOutput ${EXECUTABLE_OUTPUT_PATH}/TestRootOutput )
