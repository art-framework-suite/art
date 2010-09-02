# project name
project(artFrameworkCore)

set( artFrameworkCore_sources Actions.cc
			     Breakpoints.cc
			     CachedProducts.cc
			     ConfigurableInputSource.cc
			     CurrentProcessingContext.cc
			     DataKeyTags.cc
			     DataProxyProvider.cc
			     DataViewImpl.cc
			     DelayedReader.cc
			     DependentRecordIntervalFinder.cc
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
			     IntersectingIOVRecordIntervalFinder.cc
			     IOVSyncValue.cc
			     LooperFactory.cc
			     LuminosityBlock.cc
			     LuminosityBlockPrincipal.cc
			     MakeDataException.cc
			     ModuleFactory.cc
			     NoDataException.cc
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
			     SourceFactory.cc
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

link_libraries( artPersistencyProvenance
                artPersistencyCommon
		artMessageLogger
		artFrameworkServicesBasic
		artFrameworkCore
		artParameterSet
		artFrameworkPluginManager
		artFrameworkServicesRegistry
		artUtilities
		artVersion
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PROGRAM_OPTIONS_LIBRARY}
                ${SIGC}
		${CORE}
		${REFLEX}
		${MATRIX} )

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
	      Callback.h
	      CallbackProxy.h
	      CMakeLists.txt
	      ConfigurableInputSource.h
	      ConstProductRegistry.h
	      CurrentProcessingContext.h
	      data_default_record_trait.h
	      DataKeyTags.h
	      DataProxyProvider.h
	      DataProxyTemplate.h
	      DataViewImpl.h
	      DelayedReader.h
	      DependentRecordImplementation.h
	      DependentRecordIntervalFinder.h
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
	      HCMethods.h
	      HCMethods.icc
	      HCTypeTag.h
	      HCTypeTag.icc
	      HCTypeTagTemplate.h
	      HCTypeTagTemplate.icc
	      IEventProcessor.h
	      InputSourceDescription.h
	      InputSource.h
	      InputSourceMacros.h
	      IOVSyncValue.h
	      LooperFactory.h
	      LuminosityBlock.h
	      LuminosityBlockPrincipal.h
	      MakeDataException.h
	      MakerMacros.h
	      ModuleFactory.h
	      NoDataException.h
	      NoDelayedReader.h
	      NoProxyException.h
	      NoRecordException.h
	      OccurrenceTraits.h
	      OutputModuleDescription.h
	      OutputModule.h
	      Principal.h
	      produce_helpers.h
	      ProducerBase.h
	      ProductRegistryHelper.h
	      ProxyArgumentFactoryTemplate.h
	      ProxyFactoryBase.h
	      ProxyFactoryTemplate.h
	      recordGetImplementation.icc
	      Run.h
	      RunPrincipal.h
	      Schedule.h
	      SelectorBase.h
	      Selector.h
	      SourceFactory.h
	      TFileDirectory.h
	      TH1AddDirectorySentry.h
	      TriggerNames.h
	      TriggerNamesService.h
	      TriggerReport.h
	      UnknownModuleException.h
	      UnscheduledHandler.h
	      ValidityInterval.h
	DESTINATION include/art/Framework/Core )
