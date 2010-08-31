# project name
project(FWCorePrescaleServicePlugins)

# version number
set (FWCorePrescaleServicePlugins_VERSION_MAJOR 1)
set (FWCorePrescaleServicePlugins_VERSION_MINOR 0)

set( FWCorePrescaleServicePlugins_sources Module.cc )

link_libraries( FWCorePrescaleServices
		FWCoreServiceRegistry )

add_library(FWCorePrescaleServicePlugins SHARED ${FWCorePrescaleServicePlugins_sources} )

install ( TARGETS FWCorePrescaleServicePlugins DESTINATION lib )
