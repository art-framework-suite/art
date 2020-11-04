art_make(LIBRARY_NAME art_Framework_IO_Sources ### MIGRATE-ACTION-RECOMMENDED: use art_make_library(), art_dictionary(), simple_plugin() with explicit source lists
  SUBDIRS detail
  LIB_LIBRARIES
  art_Framework_Services_FileServiceInterfaces
  art_Framework_Principal
  art_Utilities
  canvas
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
