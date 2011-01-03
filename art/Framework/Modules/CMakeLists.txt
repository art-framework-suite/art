# project name
project(art_Framework_Modules)

set( art_Framework_Modules_sources
     ProvenanceCheckerOutputModule.cc
   )

add_library(art_Framework_Modules SHARED ${art_Framework_Modules_sources} )

simple_plugin(EmptySource "plugin")
simple_plugin(Prescaler "plugin")

install ( TARGETS art_Framework_Modules
          DESTINATION ${flavorqual_dir}/lib )
