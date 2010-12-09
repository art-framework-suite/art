# project name
project(art_Framework_Services_Basic)

set( art_Framework_Services_Basic_sources Memory.cc
                                       ProfParse.cc
                                       Sym.cc
                                       Timing.cc
                                       Tracer.cc
                                       VertexTracker.cc )

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

simple_plugin(CurrentModule_plugin)
simple_plugin(EnableFloatingPointExceptions_plugin)
simple_plugin(InitRootHandlers_plugin)
simple_plugin(LoadAllDictionaries_plugin)
simple_plugin(SimpleMemoryCheck_plugin)
simple_plugin(TFileService_plugin)
simple_plugin(Timing_plugin)
simple_plugin(Tracer_plugin)
simple_plugin(UnixSignalService_plugin)

install ( TARGETS art_Framework_Services_Basic DESTINATION ${flavorqual_dir}/lib )
install(FILES CurrentModule.h
              TFileService.h
              Timing.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Basic )
