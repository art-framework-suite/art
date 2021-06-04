cet_make_library(HEADERS_TARGET SOURCE
    Breakpoints.cc
    ConsumesCollector.cc
    DecrepitRelicInputSourceImplementation.cc
    EDAnalyzer.cc
    EDFilter.cc
    EDProducer.cc
    EmptyEventTimestampPlugin.cc
    EndPathExecutor.cc
    EventSelector.cc
    FileBlock.cc
    FileCatalogMetadataPlugin.cc
    GroupSelector.cc
    GroupSelectorRules.cc
    InputSource.cc
    InputSourceFactory.cc
    InputSourceMutex.cc
    MFStatusUpdater.cc
    Modifier.cc
    ModuleBase.cc
    Observer.cc
    OutputModule.cc
    OutputWorker.cc
    Path.cc
    PathManager.cc
    PathsInfo.cc
    ProcessAndEventSelectors.cc
    ProducesCollector.cc
    ProducingService.cc
    ProductRegistryHelper.cc
    RPManager.cc
    ReplicatedAnalyzer.cc
    ReplicatedFilter.cc
    ReplicatedProducer.cc
    ResultsProducer.cc
    Schedule.cc
    SharedAnalyzer.cc
    SharedFilter.cc
    SharedProducer.cc
    TriggerPathsExecutor.cc
    TriggerResultInserter.cc
    UpdateOutputCallbacks.cc
    WorkerInPath.cc
    detail/Analyzer.cc
    detail/EnabledModules.cc
    detail/EngineCreator.cc
    detail/Filter.cc
    detail/LegacyModule.cc
    detail/ModuleGraphInfo.cc
    detail/ModuleGraphInfoMap.cc
    detail/ModuleKeyAndType.cc
    detail/Producer.cc
    detail/RegexMatch.cc
    detail/SharedModule.cc
    detail/consumed_products.cc
    detail/graph_algorithms.cc
    detail/issue_reports.cc
    detail/skip_non_replicated.cc
    detail/verify_names.cc
  LIBRARIES
  PUBLIC
    art::Framework_Services_Optional_RandomNumberGenerator_service
    art::Framework_Services_System_FileCatalogMetadata_service
    art::Framework_Services_System_TriggerNamesService_service
    art::Framework_Services_FileServiceInterfaces
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Persistency_Provenance
    art::Persistency_Common
    art::Utilities
    art::Version
    messagefacility::MF_MessageLogger
    canvas::canvas
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    art_plugin_support::support_macros
    art_plugin_support::plugin_config_macros
    hep_concurrency::hep_concurrency
    cetlib_except::cetlib_except
  PRIVATE
    art_plugin_types::serviceDeclaration
    cetlib::container_algorithms
    hep_concurrency::macros
    Boost::graph
    Range::Range
    Boost::json
)

cet_make_library(LIBRARY_NAME module INTERFACE
  EXPORT_SET PluginTypes SOURCE ModuleMacros.h
  LIBRARIES INTERFACE
    art::Framework_Core
    art::Framework_Principal
    art::Persistency_Provenance
    art_plugin_support::plugin_config_macros
    art_plugin_support::support_macros
    fhiclcpp::fhiclcpp
)

foreach (prefix ED Replicated Shared "")
  foreach (type Analyzer Filter Producer)
    cet_make_library(LIBRARY_NAME ${prefix}${type} INTERFACE
      EXPORT_SET PluginTypes SOURCE ${prefix}${type}.h
      LIBRARIES INTERFACE art_plugin_types::module
    )
  endforeach()
endforeach()

cet_make_library(LIBRARY_NAME Output INTERFACE
  EXPORT_SET PluginTypes SOURCE OutputModule.h
  LIBRARIES_INTERFACE art_plugin_types::module
)

cet_make_library(LIBRARY_NAME ResultsProducer INTERFACE
  EXPORT_SET PluginTypes SOURCE ResultsProducer.h
  LIBRARIES INTERFACE
    art::Framework_Core
    art::Persistency_Provenance
    art_plugin_support::plugin_config_macros
    art_plugin_support::support_macros
    fhiclcpp::fhiclcpp
)

cet_make_library(LIBRARY_NAME EmptyEventTimestampPlugin INTERFACE
  EXPORT_SET PluginTypes SOURCE EmptyEventTimestampPlugin.h
  LIBRARIES INTERFACE
  art::Framework_Core
  canvas::canvas
  art_plugin_support::plugin_config_macros
  art_plugin_support::support_macros
  fhiclcpp::fhiclcpp
)

cet_make_library(LIBRARY_NAME FileCatalogMetadataPlugin INTERFACE
  EXPORT_SET PluginTypes SOURCE FileCatalogMetadataPlugin.h
  LIBRARIES INTERFACE
  art::Framework_Core
  art::Framework_Services_System_FileCatalogMetadata_service
  art_plugin_support::plugin_config_macros
  art_plugin_support::support_macros
)

cet_make_library(LIBRARY_NAME InputSource INTERFACE
  EXPORT_SET PluginTypes SOURCE InputSourceMacros.h
  LIBRARIES INTERFACE
    art::Framework_Core
    art_plugin_support::plugin_config_macros
    art_plugin_support::support_macros
)

cet_make_library(LIBRARY_NAME ProducingService INTERFACE
  EXPORT_SET PluginTypes SOURCE ProducingService.h
  LIBRARIES INTERFACE
    art::Framework_Core
    art_plugin_types::serviceDeclaration
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
