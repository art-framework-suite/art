# project name
project(art_Framework_IO_Input)

set( art_Framework_IO_Input_sources BranchMapperWithReader.cc
                                 DuplicateChecker.cc
                                 ProvenanceAdaptor.cc
                                 RootDelayedReader.cc
                                 RootInputFile.cc
                                 RootInputFileSequence.cc
                                 RootTree.cc )

add_library(art_Framework_IO_Input SHARED ${art_Framework_IO_Input_sources} )

simple_plugin(RootInput  "source" art_Framework_IO_Input art_Framework_IO_Sources )

install ( TARGETS art_Framework_IO_Input DESTINATION ${flavorqual_dir}/lib )
