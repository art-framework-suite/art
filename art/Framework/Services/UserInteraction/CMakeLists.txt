art_make_library(LIBRARY_NAME art_Framework_Services_UserInteraction
  SOURCE UserInteraction.cc
  LIBRARIES art_Framework_Core
  art_Framework_Principal
  art_Persistency_Provenance
  art_Framework_Services_Registry
  fhiclcpp
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
