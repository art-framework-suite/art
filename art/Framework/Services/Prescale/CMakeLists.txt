# project name
project(artFrameworkServicesPrescale)

set( artFrameworkServicesPrescale_sources PrescaleService.cc )

#link_libraries( artFrameworkPluginManager
#                artUtilities
#                ${Boost_THREAD_LIBRARY} )

add_library(artFrameworkServicesPrescale SHARED ${artFrameworkServicesPrescale_sources} )

install ( TARGETS artFrameworkServicesPrescale DESTINATION ${flavorqual_dir}/lib )
install(FILES PrescaleService.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Prescale )
