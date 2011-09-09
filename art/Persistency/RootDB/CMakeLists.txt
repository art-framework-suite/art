set(art_Persistency_RootDB_suorce
  spmemvfs.c
)

link_libraries( ${SQLITE3} )
add_library(art_Persistency_RootDB SHARED ${art_Persistency_RootDB_suorce} )

install_headers()
install_source()
