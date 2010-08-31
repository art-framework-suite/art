install(FILES PrescaleService.h
	DESTINATION include/FWCore/PrescaleService/interface )
>>><<<
# project name
project(FWCorePrescaleServices)

# version number
set (FWCorePrescaleServices_VERSION_MAJOR 1)
set (FWCorePrescaleServices_VERSION_MINOR 0)

set( FWCorePrescaleServices_sources PrescaleService.cc )

link_libraries( FWCorePluginManager
                FWCoreUtilities
		${Boost_THREAD_LIBRARY} )

add_library(FWCorePrescaleServices SHARED ${FWCorePrescaleServices_sources} )

install ( TARGETS FWCorePrescaleServices DESTINATION lib )
