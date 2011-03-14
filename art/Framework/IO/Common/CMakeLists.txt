# project name
project(art_Framework_IO_Common)

#set( art_Framework_IO_Common_sources RootServiceChecker.cc )

#add_library(art_Framework_IO_Common SHARED ${art_Framework_IO_Common_sources} )

#install ( TARGETS art_Framework_IO_Common DESTINATION ${flavorqual_dir}/lib )

install( FILES ClassFiller.h
               RefStreamer.h
               #RootServiceChecker.h
               TransientStreamer.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Common
       )

install_source()
