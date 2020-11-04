art_make( ### MIGRATE-ACTION-RECOMMENDED: use art_make_library(), art_dictionary(), simple_plugin() with explicit source lists
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_System_FileCatalogMetadata_service
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
  ${TBB})

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
