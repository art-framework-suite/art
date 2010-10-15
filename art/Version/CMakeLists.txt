# project name
project(artVersion)

configure_file ( ${art_SOURCE_DIR}/art/Version/GetReleaseVersion.cc.in
                 ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc @ONLY )

set( artVersion_sources GetFileFormatVersion.cc
                        ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc )

add_library(artVersion SHARED ${artVersion_sources} )

install ( TARGETS artVersion DESTINATION ${flavorqual_dir}/lib )
install(FILES GetFileFormatVersion.h
              GetReleaseVersion.h
        DESTINATION ${product}/${version}/include/art/Version )
