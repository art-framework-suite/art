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
	DESTINATION include/FWCore/PluginManager/interface )
>>><<<
# project name
project(FWCorePluginManager)

# version number
set (FWCorePluginManager_VERSION_MAJOR 1)
set (FWCorePluginManager_VERSION_MINOR 0)

set( FWCorePluginManager_sources CacheParser.cc
				 FileOperations.cc
				 PluginCapabilities.cc
				 PluginFactoryBase.cc
				 PluginFactoryManager.cc
				 PluginManager.cc
				 PresenceFactory.cc
				 ProblemTracker.cc
				 SharedLibrary.cc
				 standard.cc )

link_libraries( FWCoreUtilities
                ${Boost_FILESYSTEM_LIBRARY}
                ${SIGC} )

add_library(FWCorePluginManager SHARED ${FWCorePluginManager_sources} )

install ( TARGETS FWCorePluginManager DESTINATION lib )
