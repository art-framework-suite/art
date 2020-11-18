set(art_Framework_EventProcessor_sources
  EventProcessor.cc
  Scheduler.cc
  detail/ExceptionCollector.cc
  detail/writeSummary.cc)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND art_Framework_EventProcessor_sources detail/memoryReportLinux.cc)
else()
  list(APPEND art_Framework_EventProcessor_sources detail/memoryReportDarwin.cc)
endif()

art_make_library(
  SOURCE
  ${art_Framework_EventProcessor_sources}
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
