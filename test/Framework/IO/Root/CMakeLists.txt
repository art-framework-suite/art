link_libraries (art_Framework_IO_RootVersion)
cet_test(GetFileFormatVersion SOURCES test_GetFileFormatVersion.cpp)


########################################################################
# Old (pre-fork) tests -- not yet vetted / reactivated.
if (0)
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

add_executable( TestRootInput TestRootInput.cpp )
add_test( TestRootInput ${EXECUTABLE_OUTPUT_PATH}/TestRootInput )

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
endif(0)
