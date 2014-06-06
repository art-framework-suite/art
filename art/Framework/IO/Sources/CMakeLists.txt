art_make(LIBRARY_NAME art_Framework_IO_Sources
  SUBDIRS detail
  LIB_LIBRARIES art_Framework_Services_FileServiceInterfaces
  art_Framework_Services_Registry
  art_Framework_Principal
  art_Persistency_Common
  art_Persistency_Provenance
  art_Utilities
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
