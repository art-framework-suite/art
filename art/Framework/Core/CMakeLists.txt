art_make(
  SUBDIRS detail
  EXCLUDE
  check_data_dependencies.cc
  LIB_LIBRARIES
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional_RandomNumberGenerator_service
  art_Framework_Principal
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  MF_MessageLogger
  fhiclcpp
  canvas
  cetlib
  hep_concurrency
  ${CLHEP}
  ${TBB}
  )

cet_make_exec(check_data_dependencies SOURCE check_data_dependencies.cc LIBRARIES canvas cetlib_except art_Framework_Core)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
