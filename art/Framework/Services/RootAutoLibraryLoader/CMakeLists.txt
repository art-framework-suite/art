# project name
project(artFrameworkServicesRootAutoLibraryLoader)

set( artFrameworkServicesRootAutoLibraryLoader_sources RootAutoLibraryLoader.cc
                                                       stdNamespaceAdder.cc )

#link_libraries( artFrameworkPluginManager
#                DataFormatsProvenance
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_REGEX_LIBRARY}
#                ${CORE}
#                ${REFLEX}
#                ${CINT} dl )

add_library(artFrameworkServicesRootAutoLibraryLoader SHARED ${artFrameworkServicesRootAutoLibraryLoader_sources} )

install ( TARGETS artFrameworkServicesRootAutoLibraryLoader DESTINATION lib )
install(FILES CacheStreamers.h
              RootAutoLibraryLoader.h
        DESTINATION include/art/Framework/Services/RootAutoLibraryLoader )
