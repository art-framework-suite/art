# project name
project(artFrameworkServicesPrescale)

set( artFrameworkServicesPrescale_sources PrescaleService.cc )

link_libraries( artFrameworkPluginManager
                artUtilities
		${Boost_THREAD_LIBRARY} )

add_library(artFrameworkServicesPrescale SHARED ${artFrameworkServicesPrescale_sources} )

install ( TARGETS artFrameworkServicesPrescale DESTINATION lib )
install(FILES PrescaleService.h
	DESTINATION include/art/Framework/Services/Prescale )
