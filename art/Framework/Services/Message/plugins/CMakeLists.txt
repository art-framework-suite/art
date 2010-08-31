# project name
project(FWCoreMessageServicePlugins)

# version number
set (FWCoreMessageServicePlugins_VERSION_MAJOR 1)
set (FWCoreMessageServicePlugins_VERSION_MINOR 0)

set( FWCoreMessageServicePlugins_sources Module.cc )

link_libraries( FWCoreMessageServices
		FWCoreServiceRegistry
		FWCorePluginManager )

add_library(FWCoreMessageServicePlugins SHARED ${FWCoreMessageServicePlugins_sources} )

install ( TARGETS FWCoreMessageServicePlugins DESTINATION lib )
