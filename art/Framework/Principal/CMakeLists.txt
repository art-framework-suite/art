art_make(LIB_LIBRARIES art_Persistency_Provenance
  art_Persistency_Common
  art_Framework_Services_Registry
  art_Utilities
  art_Version
  MF_MessageLogger
  fhiclcpp
  cetlib
  ${CLHEP}
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
