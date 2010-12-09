# project name
project(art_Framework_IO_Output)

set( art_Framework_IO_Output_sources RootOutputFile.cc
                                  RootOutputTree.cc )

#link_libraries( art_Framework_IO_Common
#                artPersistencyCommon
#                artPersistencyProvenance
#                art_Framework_Services_Registry
#                art_ParameterSet
#                artMessageLogger
#                art_Framework_Core
#                art_Utilities
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#                ${REFLEX}
#                ${CORE} )

add_library(art_Framework_IO_Output SHARED ${art_Framework_IO_Output_sources} )

simple_plugin(PoolOutputModule_plugin)

install ( TARGETS art_Framework_IO_Output DESTINATION ${flavorqual_dir}/lib )
