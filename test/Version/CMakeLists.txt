# project name
project(art_Version_Test)

configure_file ( ${art_SOURCE_DIR}/test/Version/test_GetReleaseVersion.cpp.in
                 ${art_BINARY_DIR}/test/Version/test_GetReleaseVersion.cpp @ONLY )

set( cpp_files test_GetFileFormatVersion.cpp
	       ${art_BINARY_DIR}/test/Version/test_GetReleaseVersion.cpp )

link_libraries(  art_Version  )

add_executable( test_GetFileFormatVersion test_GetFileFormatVersion.cpp )
add_executable( test_GetReleaseVersion ${art_BINARY_DIR}/test/Version/test_GetReleaseVersion.cpp )

# define the tests
# use "make test" to run them
add_test(GetFileFormatVersion ${EXECUTABLE_OUTPUT_PATH}/test_GetFileFormatVersion)
add_test(GetReleaseVersion    ${EXECUTABLE_OUTPUT_PATH}/test_GetReleaseVersion)

