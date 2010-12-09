# project name
project(art_Framework_PluginManager)

# version number
set (art_Framework_PluginManager_VERSION_MAJOR 1)
set (art_Framework_PluginManager_VERSION_MINOR 0)

set( art_Framework_PluginManager_sources CacheParser.cc
                                       FileOperations.cc
                                       PluginCapabilities.cc
                                       PluginFactoryBase.cc
                                       PluginFactoryManager.cc
                                       PluginManager.cc
                                       PresenceFactory.cc
                                       ProblemTracker.cc
                                       SharedLibrary.cc
                                       standard.cc )

#link_libraries( art_Utilities
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${SIGC} )

add_library(art_Framework_PluginManager SHARED ${art_Framework_PluginManager_sources} )

install ( TARGETS art_Framework_PluginManager DESTINATION ${flavorqual_dir}/lib )
install(FILES CacheParser.h
              FileOperations.h
              ModuleDef.h
              PluginCapabilities.h
              PluginFactoryBase.h
              PluginFactory.h
              PluginFactoryManager.h
              PluginInfo.h
              PluginManager.h
              PresenceFactory.h
              PresenceMacros.h
              ProblemTracker.h
              SharedLibrary.h
              standard.h
         DESTINATION  ${product}/${version}/include/art/Framework/PluginManager )
