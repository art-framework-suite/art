# project name
project(artFrameworkServicesRegistry)

set( artFrameworkServicesRegistry_sources ActivityRegistry.cc
                                          ServiceMakerBase.cc
                                          ServicePluginFactory.cc
                                          ServiceRegistry.cc
                                          ServicesManager.cc
                                          ServiceToken.cc
                                          ServiceWrapperBase.cc )

#link_libraries( artUtilities
#                artParameterSet
#                artFrameworkPluginManager
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${SIGC} )

add_library(artFrameworkServicesRegistry SHARED ${artFrameworkServicesRegistry_sources} )

install ( TARGETS artFrameworkServicesRegistry DESTINATION lib )
install(FILES ActivityRegistry.h
              connect_but_block_self.h
              Service.h
              ServiceLegacy.h
              ServiceMakerBase.h
              ServiceMaker.h
              ServicePluginFactory.h
              ServiceRegistry.h
              ServicesManager.h
              ServiceToken.h
              ServiceWrapperBase.h
              ServiceWrapper.h
              TypeInfoHolder.h
        DESTINATION include/art/Framework/Services/Registry )
