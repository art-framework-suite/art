# project name
project(art_Framework_IO_Input)

set( art_Framework_IO_Input_sources BranchMapperWithReader.cc
                                 DuplicateChecker.cc
                                 Module.cc
                                 PoolSource.cc
                                 ProvenanceAdaptor.cc
                                 RootDelayedReader.cc
                                 RootFile.cc
                                 RootInputFileSequence.cc
                                 RootTree.cc )

add_library(art_Framework_IO_Input SHARED ${art_Framework_IO_Input_sources} )

simple_plugin(PoolRASource)
simple_plugin(PoolSource)

install ( TARGETS art_Framework_IO_Input DESTINATION ${flavorqual_dir}/lib )
