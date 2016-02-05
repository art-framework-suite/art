set( art_Persistency_Provenance_libraries
  art_Persistency_RootDB
  art_Utilities canvas_Utilities
  MF_MessageLogger
  ${ROOT_CORE}
  ${Boost_THREAD_LIBRARY}
  )

art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Persistency_RootDB
  art_Utilities canvas_Utilities
  ${Boost_THREAD_LIBRARY}
  MF_MessageLogger
	)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
