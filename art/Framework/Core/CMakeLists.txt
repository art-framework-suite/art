# project name
project(art_Framework_Core)

foreach(cfg_source
    LibraryManager.h
    LibraryManager.cc
    check_libs.cc
    )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${cfg_source}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${cfg_source} @ONLY
    )
endforeach()

set( art_Framework_Core_sources
  Actions.cc
  Breakpoints.cc
  CachedProducts.cc
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
  GenericHandle.cc
  Group.cc
  GroupSelector.cc
  GroupSelectorRules.cc
  IEventProcessor.cc
  InputSource.cc
  InputSourceFactory.cc
  IntermediateTablePostProcessor.cc
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager.cc
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
  wrapLibraryManagerException.cc
)

simple_plugin(RandomNumberSaver  "module")

add_library(art_Framework_Core SHARED ${art_Framework_Core_sources} )
target_link_libraries(art_Framework_Core
  art_Framework_Services_System_ConstProductRegistry_service
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_System_TriggerNamesService_service
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
set( art_Framework_CoreArt_sources
  NovaConfigPostProcessor.cc
  artapp.cc
  find_config.cc
  novaapp.cc
  run_art.cc
  )

add_library(art_Framework_CoreArt SHARED ${art_Framework_CoreArt_sources} )

set ( common_exec_link_libraries 
  art_Framework_Core
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${CINTEX}
  ${PHYSICS}
  ${ROOTSYS}/lib/libGraf.so
  ${TREE}
  ${HIST}
  ${MATRIX}
  ${NET}
  ${MATHCORE}
  ${THREAD}
  ${RIO}
	${CORE}
  ${CINT}
	${REFLEX}
  ${CPPUNIT}
  -ldl
)

set ( art_exec_link_libraries
  art_Persistency_Common_dict
  art_Framework_CoreArt
  art_Framework_Services_Registry
  art_Utilities
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
)

set (ART_EXECS
  art
  lar
  nova
)

foreach (art_exec ${ART_EXECS})
  add_executable( ${art_exec} ${art_exec}.cc )
  target_link_libraries( ${art_exec}
                         ${art_exec_link_libraries}
                         ${common_exec_link_libraries} )
  install ( TARGETS ${art_exec} DESTINATION ${flavorqual_dir}/bin )
endforeach (art_exec ${ART_EXECS})

add_executable( check_libs ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc )
target_link_libraries( check_libs ${common_exec_link_libraries} )
install ( TARGETS check_libs DESTINATION ${flavorqual_dir}/bin )

install ( TARGETS art_Framework_Core DESTINATION ${flavorqual_dir}/lib )
install ( TARGETS art_Framework_CoreArt DESTINATION ${flavorqual_dir}/lib )

install(FILES
	TypeLabelList.h
  Actions.h
  BranchActionType.h
  Breakpoints.h
  CPCSentry.h
  CachedProducts.h
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
  GenericHandle.h
  Group.h
  GroupSelector.h
  GroupSelectorRules.h
  IEventProcessor.h
  InputSource.h
  InputSourceDescription.h
  InputSourceFactory.h
  InputSourceMacros.h
  IntermediateTablePostProcessor.h
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager.h
  ModuleFactory.h
  ModuleMacros.h
  NoDelayedReader.h
  NovaConfigPostProcessor.h
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
  View.h
  Worker.h
  WorkerInPath.h
  WorkerParams.h
  WorkerRegistry.h
  WorkerT.h
  find_config.h
  run_art.h
  DESTINATION  ${product}/${version}/include/art/Framework/Core )
