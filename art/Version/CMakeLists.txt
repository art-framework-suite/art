# project name
project(art_Version)

include(ArtMake)

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/GetReleaseVersion.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc @ONLY )

art_make_library( LIBRARY_NAME art_Version
                  SOURCE ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc
                 )

install_headers()
install_source()
