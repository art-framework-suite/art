set( art_Persistency_Provenance_libraries
  art_Persistency_RootDB
  art_Utilities
  ${Boost_THREAD_LIBRARY}
  ${ROOT_CINTEX}
  MF_MessageLogger
  )

art_make(SUBDIRS detail
  LIB_LIBRARIES
  canvas_Persistency_Provenance
  art_Persistency_RootDB
  art_Utilities
  ${Boost_THREAD_LIBRARY}
  ${ROOT_CINTEX}
  MF_MessageLogger
        )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
