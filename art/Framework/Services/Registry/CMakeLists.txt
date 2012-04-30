# project name
project(art_Framework_Services_Registry)

set( art_Framework_Services_Registry_sources ActivityRegistry.cc
                                          ServiceRegistry.cc
                                          ServicesManager.cc
                                          ServiceWrapperBase.cc )

add_library(art_Framework_Services_Registry SHARED ${art_Framework_Services_Registry_sources} )

link_libraries(art_Utilities)

install ( TARGETS art_Framework_Services_Registry DESTINATION ${flavorqual_dir}/lib )
install_headers(SUBDIRS detail)
install_source()
