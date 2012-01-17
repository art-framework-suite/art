# project name
project(art_Version)

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/GetReleaseVersion.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc @ONLY )

add_library(art_Version SHARED
  ${CMAKE_CURRENT_BINARY_DIR}/GetReleaseVersion.cc
  )

install ( TARGETS art_Version DESTINATION ${flavorqual_dir}/lib )
install(FILES
  GetReleaseVersion.h
  DESTINATION ${product}/${version}/include/art/Version )

install_source()
