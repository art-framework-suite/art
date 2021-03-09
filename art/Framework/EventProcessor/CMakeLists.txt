cet_make_library(SOURCE
    EventProcessor.cc
    Scheduler.cc
    detail/ExceptionCollector.cc
    detail/writeSummary.cc
    detail/memoryReport${CMAKE_SYSTEM_NAME}.cc
  LIBRARIES
  PUBLIC
    art::Framework_Core
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Utilities
    canvas::canvas
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    hep_concurrency::thread_sanitize
    TBB::tbb
  PRIVATE
    art::Framework_Services_Optional_RandomNumberGenerator_service
    art::Framework_Services_System_FileCatalogMetadata_service
    art::Framework_Services_System_FloatingPointControl_service
    art::Framework_Services_System_TriggerNamesService_service
    art::Persistency_Provenance
    art::Version
    canvas::canvas
    messagefacility::MF_MessageLogger
    cetlib::container_algorithms
    hep_concurrency::hep_concurrency
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
