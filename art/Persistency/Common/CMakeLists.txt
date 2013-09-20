art_make(SUBDIRS detail
  LIB_LIBRARIES
	art_Persistency_Provenance
	art_Utilities
	${ROOT_CORE}
	${Boost_THREAD_LIBRARY}
  DICT_LIBRARIES
  art_Persistency_Common
	)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
