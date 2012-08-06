# project name
project(art_Framework_Services_Registry)

set( art_Framework_Services_Registry_sources ActivityRegistry.cc
                                          ServiceRegistry.cc
                                          ServicesManager.cc
                                          ServiceWrapperBase.cc )

add_library(art_Framework_Services_Registry SHARED ${art_Framework_Services_Registry_sources} )

target_link_libraries(art_Framework_Services_Registry
  art_Utilities
  ${SIGC}
)

install ( TARGETS art_Framework_Services_Registry DESTINATION ${flavorqual_dir}/lib )
install(FILES ActivityRegistry.h
              connect_but_block_self.h
              ServiceHandle.h
              ServiceMacros.h
              ServiceRegistry.h
              ServicesManager.h
              ServiceToken.h
              ServiceWrapperBase.h
              ServiceWrapper.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Registry )

install_source()
