set_source_files_properties(
  test2.cc
  PROPERTIES
  COMPILE_DEFINITIONS TKEYVFS_DO_ROOT
  )

cet_test(tkeyvfs_t_01 NO_AUTO
  SOURCES test1.c myvfs.c
  LIBRARIES ${SQLITE3} -ldl
  )

cet_test(tkeyvfs_t_02 NO_AUTO
  SOURCES test2.cc
  LIBRARIES art_Persistency_RootDB ${RIO} ${THREAD} ${CORE} ${CINT} ${SQLITE3} -ldl
)

cet_test(tkeyvfs_t_03 NO_AUTO
  SOURCES test3.cc tkeyvfs_noroot.cc
  LIBRARIES ${SQLITE3} -ldl
)
