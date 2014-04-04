link_libraries (art_Framework_IO_RootVersion)
cet_test(GetFileFormatVersion SOURCES test_GetFileFormatVersion.cpp)

foreach (mode M S P)
  cet_test(config_dumper_${mode}_t HANDBUILT
    TEST_EXEC config_dumper
    TEST_ARGS -${mode} ../../../../Integration/Assns_w.d/out.root
    REF "${CMAKE_CURRENT_SOURCE_DIR}/config_dumper_${mode}_t-ref.txt"
    TEST_PROPERTIES
    DEPENDS Assns_w
    REQUIRED_FILES ../../../../Integration/Assns_w.d/out.root
    )
endforeach()

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
  ${ROOT_CORE}
	${ROOT_REFLEX}
	${ROOT_CINTEX}
	${ROOT_CINT}
	${ROOT_RIO}
	${ROOT_NET}
	${ROOT_TREE}
	${CPPUNIT}
	-ldl )

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
  ${ROOT_CORE}
	${ROOT_REFLEX}
	${ROOT_CINTEX}
	${ROOT_CINT}
	${ROOT_RIO}
	${ROOT_NET}
	${ROOT_TREE}
	${CPPUNIT}
	-ldl )

add_executable( TestRootOutput TestRootOutput.cpp )
add_test( TestRootOutput ${EXECUTABLE_OUTPUT_PATH}/TestRootOutput )
endif(0)
