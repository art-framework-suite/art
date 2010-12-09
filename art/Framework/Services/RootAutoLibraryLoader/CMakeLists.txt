# project name
project(art_Framework_Services_RootAutoLibraryLoader)

set( art_Framework_Services_RootAutoLibraryLoader_sources RootAutoLibraryLoader.cc
                                                       stdNamespaceAdder.cc )

#link_libraries( art_Framework_PluginManager
#                DataFormatsProvenance
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_REGEX_LIBRARY}
#                ${CORE}
#                ${REFLEX}
#                ${CINT} dl )

add_library(art_Framework_Services_RootAutoLibraryLoader SHARED ${art_Framework_Services_RootAutoLibraryLoader_sources} )

install ( TARGETS art_Framework_Services_RootAutoLibraryLoader DESTINATION ${flavorqual_dir}/lib )
install(FILES CacheStreamers.h
              RootAutoLibraryLoader.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/RootAutoLibraryLoader )
