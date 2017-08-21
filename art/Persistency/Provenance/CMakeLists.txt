art_make(SUBDIRS detail
  LIB_LIBRARIES
  MF_MessageLogger
  art_Utilities
  canvas
  root_Core # Required for TVersionCheck
  tbb # Required from thread_safe_registry_via_id
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
