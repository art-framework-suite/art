install(FILES CurrentModuleService.h
	      PrintLoadingPlugins.h
	      RandomNumberGeneratorService.h
	      TFileService.h
	      Timing.h
	      UpdaterService.h
	DESTINATION include/FWCore/Services/interface )
>>><<<
# project name
project(FWCoreServices)

# version number
set (FWCoreServices_VERSION_MAJOR 1)
set (FWCoreServices_VERSION_MINOR 0)

set( FWCoreServices_sources CurrentModuleService.cc
			    EnableFloatingPointExceptions.cc
			    InitRootHandlers.cc
			    LoadAllDictionaries.cc
			    LockService.cc
			    Memory.cc
			    PrintLoadingPlugins.cc
			    Profiling.cc
			    ProfParse.cc
			    RandomNumberGeneratorService.cc
			    SimpleProfiler.cc
			    Sym.cc
			    TFileService.cc
			    Timing.cc
			    Tracer.cc
			    UnixSignalService.cc
			    UpdaterService.cc
			    VertexTracker.cc )

link_libraries( DataFormatsProvenance
                DataFormatsCommon
		FWCoreServiceRegistry
		FWCoreCatalog
		FWCoreMessageLogger
		FWCoreParameterSet
		FWCorePluginManager
		FWCoreRootAutoLibraryLoader
		FWCoreUtilities
		${Boost_FILESYSTEM_LIBRARY} 
		${Boost_THREAD_LIBRARY}
		${CORE} 
		${HIST} 
		${TREE}
		${GRAF}
		${RIO}
		${NET}
		${CLHEP}
		${REFLEX} 
		${CINTEX} )

add_library(FWCoreServices SHARED ${FWCoreServices_sources} )

simple_plugin(CurrentModuleService_plugin)
simple_plugin(EnableFloatingPointExceptions_plugin)
simple_plugin(InitRootHandlers_plugin)
simple_plugin(LoadAllDictionaries_plugin)
simple_plugin(LockService_plugin)
simple_plugin(RandomNumberGeneratorService_plugin)
simple_plugin(SimpleMemoryCheck_plugin)
simple_plugin(SimpleProfiling_plugin)
simple_plugin(TFileService_plugin)
simple_plugin(Timing_plugin)
simple_plugin(Tracer_plugin)
simple_plugin(UnixSignalService_plugin)

install ( TARGETS FWCoreServices DESTINATION lib )
