art_make_library(
  SOURCE
    EventProcessor.cc
    Scheduler.cc
    detail/ExceptionCollector.cc
    detail/writeSummary.cc
    $<IF:$<PLATFORM_ID:Darwin>,detail/memoryReportDarwin.cc,detail/memoryReportLinux.cc>
  LIBRARIES PRIVATE
    art::art_Framework_Core
    art::art_Framework_Services_System_FileCatalogMetadata_service
    art::art_Framework_Services_System_FloatingPointControl_service
    art::art_Framework_Services_System_TriggerNamesService_service
    art::art_Framework_Services_Optional_RandomNumberGenerator_service
    art::art_Framework_Services_Registry
    art::art_Utilities
    messagefacility::MF_MessageLogger
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
