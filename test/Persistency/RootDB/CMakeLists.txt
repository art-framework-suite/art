set_source_files_properties(
  test3.cc
  tkeyvfs_noroot.cc
  PROPERTIES
  COMPILE_DEFINITIONS TKEYVFS_NO_ROOT
  )

cet_script(dbtest_wrap_sql NO_INSTALL)

cet_test(tkeyvfs_t_01 NO_AUTO
  SOURCES test1.c myvfs.c
  LIBRARIES ${SQLITE3} -ldl
  )

cet_test(tkeyvfs_t_01w HANDBUILT
  TEST_EXEC dbtest_wrap_sql
  TEST_ARGS -c test.db tkeyvfs_t.txt tkeyvfs_t_01 test.db
  DATAFILES tkeyvfs_t.txt
  )

cet_test(tkeyvfs_t_01r HANDBUILT
  TEST_EXEC tkeyvfs_t_01
  TEST_ARGS ../tkeyvfs_t_01w.d/test.db "select * from T1"
  )

SET_TESTS_PROPERTIES(tkeyvfs_t_01r PROPERTIES PASS_REGULAR_EXPRESSION "dob: 2011-09-12")

SET_TESTS_PROPERTIES(tkeyvfs_t_01r PROPERTIES DEPENDS tkeyvfs_t_01w)

cet_test(tkeyvfs_t_02 NO_AUTO
  SOURCES test2.cc
  LIBRARIES art_Persistency_RootDB
  )

cet_test(tkeyvfs_t_02w HANDBUILT
  TEST_EXEC dbtest_wrap_sql
  TEST_ARGS -c test.db tkeyvfs_t.txt tkeyvfs_t_02 w test.db test_02
  DATAFILES tkeyvfs_t.txt
  )

cet_test(tkeyvfs_t_02r HANDBUILT
  TEST_EXEC tkeyvfs_t_02
  TEST_ARGS r ../tkeyvfs_t_02w.d/test.db test_02 "select * from T1"
  )

SET_TESTS_PROPERTIES(tkeyvfs_t_02r PROPERTIES PASS_REGULAR_EXPRESSION "dob: 2011-09-12")

SET_TESTS_PROPERTIES(tkeyvfs_t_02r PROPERTIES DEPENDS tkeyvfs_t_02w)

cet_test(tkeyvfs_t_03 NO_AUTO
  SOURCES test3.cc tkeyvfs_noroot.cc
  LIBRARIES ${SQLITE3} -ldl
  )

cet_test(tkeyvfs_t_03w HANDBUILT
  TEST_EXEC dbtest_wrap_sql
  TEST_ARGS -c test.db tkeyvfs_t.txt tkeyvfs_t_03 w test.db test_03
  DATAFILES tkeyvfs_t.txt
  )

cet_test(tkeyvfs_t_03r HANDBUILT
  TEST_EXEC tkeyvfs_t_03
  TEST_ARGS r ../tkeyvfs_t_03w.d/test.db test_03 "select * from T1"
  )

SET_TESTS_PROPERTIES(tkeyvfs_t_03r PROPERTIES PASS_REGULAR_EXPRESSION "dob: 2011-09-12")

SET_TESTS_PROPERTIES(tkeyvfs_t_03r PROPERTIES DEPENDS tkeyvfs_t_03w)
