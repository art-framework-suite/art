
execute_process( COMMAND date
                 OUTPUT_VARIABLE datime
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

# create package configuration and version files

process_ups_files()

configure_file ( ${art_SOURCE_DIR}/ups/art-config.cmake.in
                 ${art_BINARY_DIR}/ups/art-config.cmake @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art-config-version.cmake.in
                 ${art_BINARY_DIR}/ups/art-config-version.cmake @ONLY )

install (FILES ${art_BINARY_DIR}/ups/art-config.cmake
               ${art_BINARY_DIR}/ups/art-config-version.cmake
	 DESTINATION ${flavorqual_dir}/lib/art-${version} )

