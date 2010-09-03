# project name
project(artFrameworkServicesMessagePlugins)

set( artFrameworkServicesMessagePlugins_sources Module.cc )

#link_libraries( artFrameworkServicesMessage
#		artFrameworkServicesRegistry
#		artFrameworkPluginManager )

add_library(artFrameworkServicesMessagePlugins SHARED ${artFrameworkServicesMessagePlugins_sources} )

install ( TARGETS artFrameworkServicesMessagePlugins DESTINATION lib )
