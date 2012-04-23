
# create package configuration and version files

process_ups_files()

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/art-config.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/art-config.cmake @ONLY )
configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/art-config-version.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/art-config-version.cmake @ONLY )

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/art-config.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/art-config-version.cmake
	 DESTINATION ${flavorqual_dir}/lib/art-${version} )

