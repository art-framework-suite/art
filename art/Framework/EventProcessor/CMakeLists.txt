art_make(
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_FileCatalogMetadata_service
  art_Framework_Services_System_FloatingPointControl_service
  art_Framework_Services_System_PathSelection_service
  art_Framework_Services_System_ScheduleContext_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Core
  art_Utilities
  ${CETLIB}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
