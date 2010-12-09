# project name
project(art_Framework_Services_Prescale)

#set( art_Framework_Services_Prescale_sources PrescaleService_plugin.cc )

#link_libraries( art_Framework_PluginManager
#                art_Utilities
#                ${Boost_THREAD_LIBRARY} )

#add_library(art_Framework_Services_Prescale SHARED ${art_Framework_Services_Prescale_sources} )

simple_plugin(PrescaleService_plugin)

#install ( TARGETS art_Framework_Services_Prescale DESTINATION ${flavorqual_dir}/lib )
#install(FILES PrescaleService.h
#         DESTINATION  ${product}/${version}/include/art/Framework/Services/Prescale )
