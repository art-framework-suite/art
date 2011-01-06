project(IORootInputTest)

set( cpp_files TestRootInput.cpp )

link_libraries( IORootInput
                IORootCommon
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

add_executable( TestRootInput TestRootInput.cpp )
add_test( TestRootInput ${EXECUTABLE_OUTPUT_PATH}/TestRootInput )
