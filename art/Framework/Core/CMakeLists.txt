# project name
project(artFrameworkCore)

set( artFrameworkCore_sources Actions.cc
			     Breakpoints.cc
			     CachedProducts.cc
			     ConfigurableInputSource.cc
			     CurrentProcessingContext.cc
			     DataViewImpl.cc
			     DelayedReader.cc
			     EDAnalyzer.cc
			     EDFilter.cc
			     EDLooper.cc
			     EDProducer.cc
			     EPStates.cc
			     Event.cc
			     EventPrincipal.cc
			     EventProcessor.cc
			     EventSelector.cc
			     Factory.cc
			     GeneratedInputSource.cc
			     GenericHandle.cc
			     GenericObjectOwner.cc
			     Group.cc
			     GroupSelector.cc
			     GroupSelectorRules.cc
			     IEventProcessor.cc
			     InputSource.cc
			     InputSourceFactory.cc
			     IOVSyncValue.cc
			     SubRun.cc
			     SubRunPrincipal.cc
			     NoDelayedReader.cc
			     OutputModule.cc
			     OutputWorker.cc
			     Path.cc
			     PrincipalCache.cc
			     Principal.cc
			     ProducerBase.cc
			     ProductRegistryHelper.cc
			     Run.cc
			     RunPrincipal.cc
			     Schedule.cc
			     SelectorBase.cc
			     Selector.cc
			     SignallingProductRegistry.cc
			     TFileDirectory.cc
			     TH1AddDirectorySentry.cc
			     TriggerNames.cc
			     TriggerNamesService.cc
			     TriggerResultInserter.cc
			     ValidityInterval.cc
			     Worker.cc
			     WorkerInPath.cc
			     WorkerMaker.cc
			     WorkerRegistry.cc )

#link_libraries( artPersistencyProvenance
#                artPersistencyCommon
#		artMessageLogger
#		artFrameworkServicesBasic
#		artFrameworkCore
#		artParameterSet
#		artFrameworkPluginManager
#		artFrameworkServicesRegistry
#		artUtilities
#		artVersion
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_PROGRAM_OPTIONS_LIBRARY}
#                ${SIGC}
#		${CORE}
#		${REFLEX}
#		${MATRIX} )

add_library(artFrameworkCore SHARED ${artFrameworkCore_sources} )

set( artFrameworkCoreArt_sources artapp.cc )

add_library(artFrameworkCoreArt SHARED ${artFrameworkCoreArt_sources} )

add_executable( art   art.cpp   )

install ( TARGETS artFrameworkCore DESTINATION lib )
install ( TARGETS artFrameworkCoreArt DESTINATION lib )
install ( TARGETS art   DESTINATION bin )
install(FILES Actions.h
	      BranchActionType.h
	      CachedProducts.h
	      CMakeLists.txt
	      ConfigurableInputSource.h
	      ConstProductRegistry.h
	      CurrentProcessingContext.h
	      DataProxyProvider.h
	      DataProxyTemplate.h
	      DataViewImpl.h
	      DelayedReader.h
	      EDAnalyzer.h
	      EDFilter.h
	      EDLooper.h
	      EDProducer.h
	      EngineCreator.h
	      Event.h
	      EventPrincipal.h
	      EventProcessor.h
	      EventSelector.h
	      FileBlock.h
	      Frameworkfwd.h
	      GeneratedInputSource.h
	      GenericHandle.h
	      GenericObjectOwner.h
	      Group.h
	      GroupSelector.h
	      GroupSelectorRules.h
	      IEventProcessor.h
	      InputSourceDescription.h
	      InputSource.h
	      InputSourceMacros.h
	      IOVSyncValue.h
	      SubRun.h
	      SubRunPrincipal.h
	      MakerMacros.h
	      NoDelayedReader.h
	      OccurrenceTraits.h
	      OutputModuleDescription.h
	      OutputModule.h
	      Principal.h
	      ProducerBase.h
	      ProductRegistryHelper.h
	      Run.h
	      RunPrincipal.h
	      Schedule.h
	      SelectorBase.h
	      Selector.h
	      TFileDirectory.h
	      TH1AddDirectorySentry.h
	      TriggerNames.h
	      TriggerNamesService.h
	      TriggerReport.h
	      UnknownModuleException.h
	      UnscheduledHandler.h
	      ValidityInterval.h
	DESTINATION include/art/Framework/Core )
