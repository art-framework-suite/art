project(IOPoolCommonTest)

set( cpp_files TestEdmFastMerge.cpp )

link_libraries( IOPoolCommon
                DataFormatsProvenance
                FWCoreUtilities
		FWCoreServiceRegistry
		${MATRIX} )

add_executable( TestEdmFastMerge TestEdmFastMerge.cpp )
add_test( TestEdmFastMerge ${EXECUTABLE_OUTPUT_PATH}/TestEdmFastMerge )
