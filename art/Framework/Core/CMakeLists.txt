# project name
project(art_Framework_Core)

set( art_Framework_Core_sources
  Actions.cc
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
  EventObserver.cc
  EventPrincipal.cc
  EventProcessor.cc
  EventSelector.cc
  GeneratedInputSource.cc
  GenericHandle.cc
  GenericObjectOwner.cc
  Group.cc
  GroupSelector.cc
  GroupSelectorRules.cc
  IEventProcessor.cc
  InputSource.cc
  InputSourceFactory.cc
  LibraryManager.cc
  ModuleFactory.cc
  RootDictionaryManager.cc
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
  TriggerResultInserter.cc
  Worker.cc
  WorkerInPath.cc
  WorkerRegistry.cc
)

simple_plugin(RandomNumberSaver  "module")

add_library(art_Framework_Core SHARED ${art_Framework_Core_sources} )
target_link_libraries(art_Framework_Core
  art_Framework_Services_Basic_CurrentModule_service
  art_Framework_Services_Basic_FloatingPointControl_service
  art_Framework_Services_Basic_TriggerNamesService_service
  art_Persistency_Provenance
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Version
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${CLHEP}
  ${SIGC}
)
set( art_Framework_CoreArt_sources artapp.cc )

add_library(art_Framework_CoreArt SHARED ${art_Framework_CoreArt_sources} )

#add_executable( art   art.cpp   )
#target_link_libraries( art art_Framework_CoreArt
#                           ${MF_MESSAGELOGGER}
#                           ${MF_MESSAGESERVICE}
#                           ${MF_UTILITIES}
#                           ${MF_EXTENSIONS}
#                          )
#install ( TARGETS art   DESTINATION ${flavorqual_dir}/bin )

install ( TARGETS art_Framework_Core DESTINATION ${flavorqual_dir}/lib )
#install ( TARGETS art_Framework_CoreArt DESTINATION ${flavorqual_dir}/lib )

install(FILES Actions.h
              BranchActionType.h
              Breakpoints.h
              CPCSentry.h
              CachedProducts.h
              ConfigurableInputSource.h
              CurrentProcessingContext.h
              DataViewImpl.h
              DelayedReader.h
              EDAnalyzer.h
              EDFilter.h
              EDProducer.h
              EPStates.h
              EngineCreator.h
              Event.h
              EventObserver.h
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
              InputSource.h
              InputSourceDescription.h
              InputSourceFactory.h
              InputSourceMacros.h
              ModuleMacros.h
              ModuleFactory.h
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
              RootDictionaryManager.h
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
              TriggerReport.h
              TriggerResultInserter.h
              UnknownModuleException.h
              UnscheduledHandler.h
              Worker.h
              WorkerInPath.h
              WorkerParams.h
              WorkerRegistry.h
              WorkerT.h
        DESTINATION  ${product}/${version}/include/art/Framework/Core )
