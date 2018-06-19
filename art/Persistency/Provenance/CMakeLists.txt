art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Utilities
  canvas
  TBB::tbb
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
