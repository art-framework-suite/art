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

set(art_Framework_CorePrincipal_sources
  DataViewImpl.cc
  Event.cc
  EventPrincipal.cc
  Group.cc
  GroupQueryResult.cc
  NoDelayedReader.cc
  Principal.cc
  Run.cc
  RunPrincipal.cc
  Selector.cc
  SelectorBase.cc
  SubRun.cc
  SubRunPrincipal.cc
)

set( art_Framework_Core_sources
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager.cc
  Actions.cc
  Breakpoints.cc
  CachedProducts.cc
  CurrentProcessingContext.cc
  DecrepitRelicInputSourceImplementation.cc
  DelayedReader.cc
  EDAnalyzer.cc
  EDFilter.cc
  EDProducer.cc
  EngineCreator.cc
  EPStates.cc
  EventObserver.cc
  EventProcessor.cc
  EventSelector.cc
  GroupSelector.cc
  GroupSelectorRules.cc
  IEventProcessor.cc
  InputSource.cc
  InputSourceFactory.cc
  IntermediateTablePostProcessor.cc
  MasterProductRegistry.cc
  ModuleFactory.cc
  OutputModule.cc
  OutputWorker.cc
  Path.cc
  PrincipalCache.cc
  PrincipalMaker.cc
  ProducerBase.cc
  ProductMetaData.cc
  ProductRegistryHelper.cc
  RootDictionaryManager.cc
  Schedule.cc
  TFileDirectory.cc
  TH1AddDirectorySentry.cc
  TriggerNames.cc
  TriggerResultInserter.cc
  Worker.cc
  WorkerInPath.cc
  WorkerRegistry.cc
  detail/BranchIDListHelper.cc
  get_BranchDescription.cc
  wrapLibraryManagerException.cc
)

add_library(art_Framework_Core SHARED ${art_Framework_Core_sources} )
target_link_libraries(art_Framework_Core
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_CorePrincipal
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

add_library(art_Framework_CorePrincipal SHARED ${art_Framework_CorePrincipal_sources})
target_link_libraries(art_Framework_CorePrincipal
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
  gm2
  lar
  mu2e
  nova
)

foreach (art_exec ${ART_EXECS})
  add_executable( ${art_exec} ${art_exec}.cc )
  target_link_libraries( ${art_exec}
                         ${art_exec_link_libraries}
                         ${common_exec_link_libraries} )
  install ( TARGETS ${art_exec} DESTINATION ${flavorqual_dir}/bin )
endforeach (art_exec ${ART_EXECS})

# Versions of the ART execs supporting modules using BOOST unit test framework.
macro(art_ut_exec TARGET_STEM MAIN_FUNC)
  set(ART_UT_MAIN_FUNC ${MAIN_FUNC})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/art_ut.cc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}_ut.cc @ONLY
    )
  add_executable(${TARGET_STEM}_ut ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}_ut.cc)
  target_link_libraries(${TARGET_STEM}_ut
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${art_exec_link_libraries}
    ${common_exec_link_libraries} )
  install ( TARGETS ${TARGET_STEM}_ut DESTINATION ${flavorqual_dir}/bin )
  set_target_properties(${TARGET_STEM}_ut PROPERTIES
    COMPILE_DEFINITIONS "BOOST_TEST_DYN_LINK"
    )
endmacro()

art_ut_exec(art artapp)
art_ut_exec(nova novaapp)
art_ut_exec(mu2e novaapp)
art_ut_exec(gm2 novaapp)

add_executable( check_libs ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc )
target_link_libraries( check_libs ${common_exec_link_libraries} )
install ( TARGETS check_libs DESTINATION ${flavorqual_dir}/bin )

install ( TARGETS
  art_Framework_Core
  art_Framework_CoreArt
  art_Framework_CorePrincipal
  DESTINATION
  ${flavorqual_dir}/lib
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager.h
  Actions.h
  BranchActionType.h
  Breakpoints.h
  CPCSentry.h
  CachedProducts.h
  CurrentProcessingContext.h
  DataViewImpl.h
  DecrepitRelicInputSourceImplementation.h
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
  FCPfwd.h
  FileBlock.h
  Frameworkfwd.h
  Group.h
  GroupQueryResult.h
  GroupSelector.h
  GroupSelectorRules.h
  IEventProcessor.h
  InputSource.h
  InputSourceDescription.h
  InputSourceFactory.h
  InputSourceMacros.h
  IntermediateTablePostProcessor.h
  MasterProductRegistry.h
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
  PrincipalMaker.h
  ProcessingTask.h
  ProducerBase.h
  ProductMetaData.h
  ProductRegistryHelper.h
  PtrRemapper.h
  RootDictionaryManager.h
  Run.h
  RunPrincipal.h
  RunStopwatch.h
  Schedule.h
  Selector.h
  SelectorBase.h
  SubRun.h
  SubRunPrincipal.h
  TFileDirectory.h
  TH1AddDirectorySentry.h
  TriggerNames.h
  TriggerReport.h
  TriggerResultInserter.h
  TypeLabelList.h
  UnknownModuleException.h
  UnscheduledHandler.h
  View.h
  Worker.h
  WorkerInPath.h
  WorkerParams.h
  WorkerRegistry.h
  WorkerT.h
  find_config.h
  get_BranchDescription.h
  run_art.h
  DESTINATION  ${product}/${version}/include/art/Framework/Core )

set(DETAIL_HEADERS
  BranchIDListHelper.h
  maybe_record_parents.h
  )

foreach (header ${DETAIL_HEADERS})
  set (PATH_DETAIL_HEADERS ${PATH_DETAIL_HEADERS} "detail/${header}")
endforeach (header ${DETAIL_HEADERS})

install (FILES ${PATH_DETAIL_HEADERS}
        DESTINATION ${product}/${version}/include/art/Framework/Core/detail )

install_source()
