art_make(LIB_LIBRARIES
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
	${FHICLCPP}
	${CETLIB}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_REGEX_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${ROOT_CINT}
	${ROOT_REFLEX}
  ${TBB}
	-lpthread
	-ldl
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
