# project name
project(art_Version)

configure_file ( ${art_SOURCE_DIR}/art/Version/GetReleaseVersion.cc.in
  ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc @ONLY )

add_library(art_Version SHARED
  ${art_BINARY_DIR}/art/Version/GetReleaseVersion.cc
  )

install ( TARGETS art_Version DESTINATION ${flavorqual_dir}/lib )
install(FILES
  GetReleaseVersion.h
  DESTINATION ${product}/${version}/include/art/Version )

install_source()
