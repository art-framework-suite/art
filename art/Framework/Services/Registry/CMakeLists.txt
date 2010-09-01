install(FILES ActivityRegistry.h
	      connect_but_block_self.h
	      Service.h
	      ServiceLegacy.h
	      ServiceMakerBase.h
	      ServiceMaker.h
	      ServicePluginFactory.h
	      ServiceRegistry.h
	      ServicesManager.h
	      ServiceToken.h
	      ServiceWrapperBase.h
	      ServiceWrapper.h
	      TypeInfoHolder.h
	DESTINATION include/FWCore/ServiceRegistry/interface )
>>><<<
# project name
project(FWCoreServiceRegistry)

# version number
set (FWCoreServiceRegistry_VERSION_MAJOR 1)
set (FWCoreServiceRegistry_VERSION_MINOR 0)

set( FWCoreServiceRegistry_sources ActivityRegistry.cc
				   ServiceMakerBase.cc
				   ServicePluginFactory.cc
				   ServiceRegistry.cc
				   ServicesManager.cc
				   ServiceToken.cc
				   ServiceWrapperBase.cc )

link_libraries( FWCoreUtilities
                FWCoreParameterSet
		FWCorePluginManager
                ${Boost_FILESYSTEM_LIBRARY}
		${SIGC} )

add_library(FWCoreServiceRegistry SHARED ${FWCoreServiceRegistry_sources} )

install ( TARGETS FWCoreServiceRegistry DESTINATION lib )
