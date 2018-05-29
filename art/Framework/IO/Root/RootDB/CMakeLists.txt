art_make_library(
  LIBRARY_NAME
  art_Framework_IO_Root_RootDB
  SOURCE
  SQLErrMsg.cc
  SQLite3Wrapper.cc
  TKeyVFSOpenPolicy.cc
  tkeyvfs.cc
  LIBRARIES
  art_Utilities
  canvas
  ${ROOT_RIO_LIBRARY}
  ${ROOT_Core_LIBRARY}
  ${SQLITE3}
  )

install_headers()
install_source()
