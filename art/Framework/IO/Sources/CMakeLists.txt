# project name
project(art_Framework_IO_Sources)

set( art_Framework_IO_Sources_sources EDInputSource.cc
  ExternalInputSource.cc
  RawInputSource.cc
  )

#link_libraries( ${Boost_FILESYSTEM_LIBRARY}  )

add_library(art_Framework_IO_Sources SHARED ${art_Framework_IO_Sources_sources} )

install ( TARGETS art_Framework_IO_Sources DESTINATION ${flavorqual_dir}/lib )
install(FILES EDInputSource.h
              ExternalInputSource.h
              RawInputSource.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Sources )
