art_make(LIB_LIBRARIES
  canvas
  MF_MessageLogger
  MF_Utilities
  fhiclcpp
  cetlib
  cetlib_except
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${ROOT_CORE}
  ${TBB}
  -lpthread
  ${CMAKE_DL_LIBS}
  SUBDIRS detail
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
