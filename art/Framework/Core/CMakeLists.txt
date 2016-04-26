art_make(
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Services_Optional_MemoryTracker_service
  art_Framework_Services_Optional_TimeTracker_service
  art_Framework_Principal
  art_Persistency_Common
  art_Persistency_Provenance canvas_Persistency_Provenance
  art_Framework_Services_Registry
  art_Utilities canvas_Utilities
  art_Version
  MF_MessageLogger
  fhiclcpp
  cetlib
  ${CLHEP}
  ${TBB}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
