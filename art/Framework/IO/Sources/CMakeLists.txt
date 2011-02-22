# project name
project(art_Framework_IO_Sources)

set( art_Framework_IO_Sources_sources EDInputSource.cc
  RawInputSource.cc
  )

#link_libraries( ${Boost_FILESYSTEM_LIBRARY}  )

add_library(art_Framework_IO_Sources SHARED ${art_Framework_IO_Sources_sources} )
target_link_libraries (art_Framework_IO_Sources art_Framework_IO_Catalog )

install ( TARGETS art_Framework_IO_Sources DESTINATION ${flavorqual_dir}/lib )
install(FILES EDInputSource.h
              RawInputSource.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Sources )
