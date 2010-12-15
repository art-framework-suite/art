# project name
project(art_Framework_Modules)

set( art_Framework_Modules_sources
     ProvenanceCheckerOutputModule.cc
   )


#link_libraries( DataFormatsProvenance
#                art_Framework_Core
#                art_ParameterSet
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${REFLEX} )

add_library(art_Framework_Modules SHARED ${art_Framework_Modules_sources} )

simple_plugin(AsciiOutputModule)
simple_plugin(EmptySource)
simple_plugin(Prescaler)

install ( TARGETS art_Framework_Modules
          DESTINATION ${flavorqual_dir}/lib )
