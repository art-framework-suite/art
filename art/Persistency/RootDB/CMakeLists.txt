set(art_Persistency_RootDB_suorce
  SQLErrMsg.cc
  SQLite3Wrapper.cc
  tkeyvfs.cc
)

art_make_library( LIBRARY_NAME art_Persistency_RootDB
                  SOURCE ${art_Persistency_RootDB_suorce}
                  LIBRARIES ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_CORE}
  ${ROOT_CINT}
  art_Utilities canvas
  ${SQLITE3}
  )

install_headers()
install_source()
