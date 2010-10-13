
execute_process( COMMAND date 
                 OUTPUT_VARIABLE datime 
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

# create package configuration and version files
configure_file ( ${art_SOURCE_DIR}/ups/art-config.cmake.in
                 ${art_BINARY_DIR}/ups/art-config.cmake @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art-config-version.cmake.in
                 ${art_BINARY_DIR}/ups/art-config-version.cmake @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art.table.in
                 ${art_BINARY_DIR}/ups/art.table @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art.version.in
                 ${art_BINARY_DIR}/ups/${version}.version @ONLY )

install (FILES ${art_BINARY_DIR}/ups/art-config.cmake
               ${art_BINARY_DIR}/ups/art-config-version.cmake
	 DESTINATION ${flavorqual_dir}/lib/art-${version} )
install (FILES ${art_BINARY_DIR}/ups/art.table
	 DESTINATION ${version}/ups )
INSTALL (FILES ${art_BINARY_DIR}/ups/${version}.version
         DESTINATION ${CMAKE_INSTALL_PREFIX} )

