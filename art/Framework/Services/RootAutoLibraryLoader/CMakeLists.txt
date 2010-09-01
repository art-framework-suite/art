install(FILES CacheStreamers.h
              RootAutoLibraryLoader.h
	DESTINATION include/FWCore/RootAutoLibraryLoader/interface )
>>><<<
# project name
project(FWCoreRootAutoLibraryLoader)

# version number
set (FWCoreRootAutoLibraryLoader_VERSION_MAJOR 1)
set (FWCoreRootAutoLibraryLoader_VERSION_MINOR 0)

set( FWCoreRootAutoLibraryLoader_sources RootAutoLibraryLoader.cc
                                         stdNamespaceAdder.cc )

link_libraries( FWCorePluginManager
                DataFormatsProvenance
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_REGEX_LIBRARY}
		${CORE}
		${REFLEX}
		${CINT} dl )

add_library(FWCoreRootAutoLibraryLoader SHARED ${FWCoreRootAutoLibraryLoader_sources} )

install ( TARGETS FWCoreRootAutoLibraryLoader DESTINATION lib )
