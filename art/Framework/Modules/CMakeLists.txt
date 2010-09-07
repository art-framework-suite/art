# project name
project(artFrameworkModules)

set( artFrameworkModules_sources
     ProvenanceCheckerOutputModule.cc
   )


#link_libraries( DataFormatsProvenance
#                artFrameworkCore
#                artParameterSet
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${REFLEX} )

add_library(artFrameworkModules SHARED ${artFrameworkModules_sources} )

simple_plugin(AsciiOutputModule_plugin)
simple_plugin(EmptySource_plugin)
simple_plugin(EventContentAnalyzer_plugin)
simple_plugin(Prescaler_plugin)
simple_plugin(XMLOutputModule_plugin)

install ( TARGETS artFrameworkModules
          DESTINATION lib )
