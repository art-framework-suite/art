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


#link_libraries( art_Framework_IO_Common
#                artPersistencyCommon
#                artPersistencyProvenance
#                art_Framework_IO_Sources
#                art_Framework_IO_Catalog
#                art_Framework_Services_Registry
#                art_ParameterSet
#                artMessageLogger
#                art_Framework_Core
#                art_Utilities
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#                ${CLHEP}
#                ${REFLEX}
#                ${CORE} )

add_library(art_Framework_IO_Input SHARED ${art_Framework_IO_Input_sources} )

simple_plugin(PoolRASource_plugin)
simple_plugin(PoolSource_plugin)

install ( TARGETS art_Framework_IO_Input DESTINATION ${flavorqual_dir}/lib )
