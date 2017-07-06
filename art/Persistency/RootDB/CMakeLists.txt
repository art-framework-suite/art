art_make_library(
  LIBRARY_NAME
  art_Persistency_RootDB
  SOURCE
  SQLErrMsg.cc
  SQLite3Wrapper.cc
  TKeyVFSOpenPolicy.cc
  tkeyvfs.cc
  LIBRARIES
  art_Utilities
  canvas
  ${ROOT_RIO}
  ${ROOT_CORE}
  ${SQLITE3}
  )

install_headers()
install_source()
