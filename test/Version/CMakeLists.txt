# project name
project(FWCoreVersionTest)

set( cpp_files test_GetFileFormatVersion.cpp
	       test_GetReleaseVersion.cpp )

link_libraries(  FWCoreVersion  )

add_executable( test_GetFileFormatVersion test_GetFileFormatVersion.cpp )
add_executable( test_GetReleaseVersion test_GetReleaseVersion.cpp )

# define the tests
# use "make test" to run them
add_test(GetFileFormatVersion ${EXECUTABLE_OUTPUT_PATH}/test_GetFileFormatVersion)
add_test(GetReleaseVersion    ${EXECUTABLE_OUTPUT_PATH}/test_GetReleaseVersion)

