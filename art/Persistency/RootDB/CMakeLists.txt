set(art_Persistency_RootDB_suorce
  MetaDataAccess.cc
  SQLErrMsg.cc
  SQLite3Wrapper.cc
  tkeyvfs.cc
)

link_libraries(
  ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_CORE}
  ${ROOT_CINT}
  art_Utilities
  ${SQLITE3}
  )

add_library(art_Persistency_RootDB SHARED ${art_Persistency_RootDB_suorce} )

install_headers()
install_source()
