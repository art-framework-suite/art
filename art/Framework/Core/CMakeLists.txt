art_make(
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_System_CurrentModule_service
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Principal
  art_Persistency_Common
  art_Persistency_Provenance canvas
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  MF_MessageLogger
  fhiclcpp
  cetlib
  canvas
  TBB::tbb
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
