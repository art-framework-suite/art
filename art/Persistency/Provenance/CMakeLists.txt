art_make(SUBDIRS detail ### MIGRATE-ACTION-RECOMMENDED: use art_make_library(), art_dictionary(), simple_plugin() with explicit source lists
  LIB_LIBRARIES
  art_Utilities
  canvas
  ${TBB}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
