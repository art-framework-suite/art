art_make(SUBDIRS detail
  LIB_LIBRARIES
  art_Utilities
  ${FHICLCPP}
  ${SIGC}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
