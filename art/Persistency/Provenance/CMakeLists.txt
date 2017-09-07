art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Utilities
  canvas
  tbb # Required from thread_safe_registry_via_id
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
