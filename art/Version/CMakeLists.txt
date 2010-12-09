# project name
project(art_Version)

configure_file ( ${art_SOURCE_DIR}/art/Version/GetReleaseVersion.cc.in
                 ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc @ONLY )

set( art_Version_sources GetFileFormatVersion.cc
                        ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc )

add_library(art_Version SHARED ${art_Version_sources} )

install ( TARGETS art_Version DESTINATION ${flavorqual_dir}/lib )
install(FILES GetFileFormatVersion.h
              GetReleaseVersion.h
        DESTINATION ${product}/${version}/include/art/Version )
