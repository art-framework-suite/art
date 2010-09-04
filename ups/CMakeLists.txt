# project name
project(artUps)

# create package configuration and version files
configure_file ( ${art_SOURCE_DIR}/ups/art-config.cmake.in
                 ${art_BINARY_DIR}/ups/art-config.cmake @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art-config-version.cmake.in
                 ${art_BINARY_DIR}/ups/art-config-version.cmake @ONLY )
configure_file ( ${art_SOURCE_DIR}/ups/art.table.in
                 ${art_BINARY_DIR}/ups/art.table @ONLY )
install (FILES ${art_BINARY_DIR}/ups/art-config.cmake
               ${art_BINARY_DIR}/ups/art-config-version.cmake
	 DESTINATION lib/art-${version} )
install (FILES ${art_BINARY_DIR}/ups/art.table
	 DESTINATION ups )

