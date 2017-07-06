art_make(LIBRARY_NAME art_Framework_IO_Sources
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_FileServiceInterfaces
  art_Framework_Principal
  art_Utilities
  canvas
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
