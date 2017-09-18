art_make(LIB_LIBRARIES
  art_Persistency_Provenance
  canvas
  canvas_root_io # Used for getWrapperTIDs
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  MF_MessageLogger
  fhiclcpp
  cetlib
  hep_concurrency
  SUBDIRS
  detail
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
