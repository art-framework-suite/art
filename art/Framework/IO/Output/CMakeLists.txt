# project name
project(artFrameworkIOOutput)

set( artFrameworkIOOutput_sources PoolOutputModule.cc
                                  RootOutputFile.cc
                                  RootOutputTree.cc )

#link_libraries( artFrameworkIOCommon
#                artPersistencyCommon
#                artPersistencyProvenance
#                artFrameworkServicesRegistry
#                artParameterSet
#                artMessageLogger
#                artFrameworkCore
#                artUtilities
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#                ${REFLEX}
#                ${CORE} )

add_library(artFrameworkIOOutput SHARED ${artFrameworkIOOutput_sources} )

simple_plugin(PoolOutputModule_plugin)

install ( TARGETS artFrameworkIOOutput DESTINATION lib )
