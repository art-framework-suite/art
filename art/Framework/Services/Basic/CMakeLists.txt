# project name
project(art_Framework_Services_Basic)

set( art_Framework_Services_Basic_sources Memory.cc
                                       Timing.cc
                                       Tracer.cc )

#link_libraries( art_Persistency_Provenance
#                art_Persistency_Common
#                art_Framework_Services_Registry
#                art_Framework_IO_Catalog
#                artMessageLogger
#                art_ParameterSet
#                art_Framework_PluginManager
#                art_Framework_Services_RootAutoLibraryLoader
#                art_Utilitiess
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#                ${CORE}
#                ${HIST}
#                ${TREE}
#                ${GRAF}
#                ${RIO}
#                ${NET}
#                ${CLHEP}
#                ${REFLEX}
#                ${CINTEX} )

add_library(art_Framework_Services_Basic SHARED ${art_Framework_Services_Basic_sources} )

simple_plugin(EnableFloatingPointExceptions)
simple_plugin(SimpleMemoryCheck)
simple_plugin(TFileService)
simple_plugin(Timing)
simple_plugin(Tracer)

install ( TARGETS art_Framework_Services_Basic DESTINATION ${flavorqual_dir}/lib )
install(FILES CurrentModule.h
              TFileService.h
              Timing.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Basic )
