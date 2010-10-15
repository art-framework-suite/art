# project name
project(artFrameworkServicesBasic)

set( artFrameworkServicesBasic_sources Memory.cc
                                       ProfParse.cc
                                       Sym.cc
                                       Timing.cc
                                       Tracer.cc
                                       VertexTracker.cc )

#link_libraries( artPersistencyProvenance
#                artPersistencyCommon
#                artFrameworkServicesRegistry
#                artFrameworkIOCatalog
#                artMessageLogger
#                artParameterSet
#                artFrameworkPluginManager
#                artFrameworkServicesRootAutoLibraryLoader
#                artUtilitiess
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

add_library(artFrameworkServicesBasic SHARED ${artFrameworkServicesBasic_sources} )

simple_plugin(CurrentModule_plugin)
simple_plugin(EnableFloatingPointExceptions_plugin)
simple_plugin(InitRootHandlers_plugin)
simple_plugin(LoadAllDictionaries_plugin)
simple_plugin(SimpleMemoryCheck_plugin)
simple_plugin(TFileService_plugin)
simple_plugin(Timing_plugin)
simple_plugin(Tracer_plugin)
simple_plugin(UnixSignalService_plugin)

install ( TARGETS artFrameworkServicesBasic DESTINATION ${flavorqual_dir}/lib )
install(FILES CurrentModule.h
              TFileService.h
              Timing.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Basic )
