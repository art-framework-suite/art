art_make(LIB_LIBRARIES
  art_Persistency_Provenance
  canvas
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  MF_MessageLogger
  fhiclcpp
  cetlib
  hep_concurrency
  ROOT_Core # necessary because Assns relies on ROOT
  SUBDIRS
  detail
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
