# project name
project(artFrameworkServicesPrescale)

#set( artFrameworkServicesPrescale_sources PrescaleService_plugin.cc )

#link_libraries( artFrameworkPluginManager
#                art_Utilities
#                ${Boost_THREAD_LIBRARY} )

#add_library(artFrameworkServicesPrescale SHARED ${artFrameworkServicesPrescale_sources} )

simple_plugin(PrescaleService_plugin)

#install ( TARGETS artFrameworkServicesPrescale DESTINATION ${flavorqual_dir}/lib )
#install(FILES PrescaleService.h
#         DESTINATION  ${product}/${version}/include/art/Framework/Services/Prescale )
