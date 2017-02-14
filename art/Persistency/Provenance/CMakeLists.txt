art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Persistency_RootDB
  art_Utilities
  canvas
  ${Boost_THREAD_LIBRARY}
  MF_MessageLogger
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
