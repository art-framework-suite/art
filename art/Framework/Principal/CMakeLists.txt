art_make(LIB_LIBRARIES ### MIGRATE-ACTION-RECOMMENDED: use art_make_library(), art_dictionary(), simple_plugin() with explicit source lists
  art_Persistency_Provenance
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  MF_MessageLogger
  fhiclcpp
  cetlib
  canvas
  hep_concurrency
  SUBDIRS
  detail
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
