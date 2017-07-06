art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Persistency_RootDB
  art_Utilities
  canvas
  MF_MessageLogger
  ${TBB} # Required from thread_safe_registry_via_id
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
