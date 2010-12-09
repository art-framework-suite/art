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
                              EDProducer.cc
                              EPStates.cc
                              Event.cc
                              EventPrincipal.cc
#                              EventProcessor.cc
                              EventSelector.cc
#                              Factory.cc
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
#                              WorkerRegistry.cc
)

link_libraries( artPersistencyProvenance
                artPersistencyCommon
                artFrameworkServicesBasic
                artFrameworkCore
                artFrameworkPluginManager
                artFrameworkServicesRegistry
                art_Utilities
                art_Version
                #artParameterSet
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PROGRAM_OPTIONS_LIBRARY}
                ${Boost_THREAD_LIBRARY}
		${Boost_REGEX_LIBRARY}
                ${SIGC}
                ${HIST}
                ${REFLEX}
		${CINT}
                ${MATRIX}
                ${MATHCORE}
                ${CORE} )

simple_plugin(RandomNumberGeneratorService_plugin)
simple_plugin(RandomNumberSaver_plugin)

add_library(artFrameworkCore SHARED ${artFrameworkCore_sources} )

set( artFrameworkCoreArt_sources artapp.cc )

#add_library(artFrameworkCoreArt SHARED ${artFrameworkCoreArt_sources} )

#add_executable( art   art.cpp   )
#target_link_libraries( art artFrameworkCoreArt
#                           ${MF_MESSAGELOGGER}
#                           ${MF_MESSAGESERVICE}
#                           ${MF_UTILITIES}
#                           ${MF_EXTENSIONS}
#			   )
#install ( TARGETS art   DESTINATION ${flavorqual_dir}/bin )

install ( TARGETS artFrameworkCore DESTINATION ${flavorqual_dir}/lib )
#install ( TARGETS artFrameworkCoreArt DESTINATION ${flavorqual_dir}/lib )

install(FILES Actions.h
	      BranchActionType.h
	      Breakpoints.h
	      CPCSentry.h
	      CachedProducts.h
	      ConfigurableInputSource.h
	      ConstProductRegistry.h
	      CurrentProcessingContext.h
	      DataViewImpl.h
	      DelayedReader.h
	      EDAnalyzer.h
	      EDFilter.h
	      EDProducer.h
	      EPStates.h
	      EngineCreator.h
	      Event.h
	      EventPrincipal.h
	      EventProcessor.h
	      EventSelector.h
	      Factory.h
	      FileBlock.h
	      Frameworkfwd.h
	      GeneratedInputSource.h
	      GenericHandle.h
	      GenericObjectOwner.h
	      Group.h
	      GroupSelector.h
	      GroupSelectorRules.h
	      IEventProcessor.h
	      IOVSyncValue.h
	      InputSource.h
	      InputSourceDescription.h
	      InputSourceFactory.h
	      InputSourceMacros.h
	      MakerMacros.h
	      NoDelayedReader.h
	      OccurrenceTraits.h
	      OutputModule.h
	      OutputModuleDescription.h
	      OutputWorker.h
	      Path.h
	      Principal.h
	      PrincipalCache.h
	      ProcessingTask.h
	      ProducerBase.h
	      ProductRegistryHelper.h
	      RandomNumberGeneratorService.h
	      Run.h
	      RunPrincipal.h
	      RunStopwatch.h
	      Schedule.h
	      Selector.h
	      SelectorBase.h
	      SignallingProductRegistry.h
	      SubRun.h
	      SubRunPrincipal.h
	      TFileDirectory.h
	      TH1AddDirectorySentry.h
	      TriggerNames.h
	      TriggerNamesService.h
	      TriggerReport.h
	      TriggerResultInserter.h
	      UnknownModuleException.h
	      UnscheduledHandler.h
	      ValidityInterval.h
	      Worker.h
	      WorkerInPath.h
	      WorkerMaker.h
	      WorkerParams.h
	      WorkerRegistry.h
	      WorkerT.h
        DESTINATION  ${product}/${version}/include/art/Framework/Core )
