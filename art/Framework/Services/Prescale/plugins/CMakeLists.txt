# project name
project(artFrameworkServicesPrescalePlugins)

set( artFrameworkServicesPrescalePlugins_sources Module.cc )

#link_libraries( artFrameworkServicesPrescale
#		artFrameworkServicesRegistry )

add_library(artFrameworkServicesPrescalePlugins SHARED ${artFrameworkServicesPrescalePlugins_sources} )

install ( TARGETS artFrameworkServicesPrescalePlugins DESTINATION lib )
