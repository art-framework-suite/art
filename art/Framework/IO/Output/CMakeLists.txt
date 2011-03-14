# project name
project(art_Framework_IO_Output)

set( art_Framework_IO_Output_sources RootOutputFile.cc
                                  RootOutputTree.cc )

add_library(art_Framework_IO_Output SHARED ${art_Framework_IO_Output_sources} )

simple_plugin(RootOutput  "module" art_Framework_IO_Output)

install ( TARGETS art_Framework_IO_Output DESTINATION ${flavorqual_dir}/lib )

install_source()
