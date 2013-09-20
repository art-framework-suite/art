art_make(SUBDIRS detail
  LIB_LIBRARIES art_Framework_Services_FileServiceInterfaces
  art_Framework_Services_Registry
  art_Utilities
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
