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
  LIBRARIES
  art_Framework_Core
  art_Framework_Services_System_FileCatalogMetadata_service
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Registry
  art_Utilities canvas
  cetlib
  hep_concurrency
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
