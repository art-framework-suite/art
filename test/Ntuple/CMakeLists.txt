cet_test(ntuple_t
  LIBRARIES art_Ntuple ${SQLITE3}
  )

cet_test( messagefacility_destination_sqlite_t HANDBUILT
  TEST_EXEC ELdestinationTester
  TEST_ARGS sqlite_mfplugin_t.fcl
  DATAFILES
  sqlite_mfplugin_t.fcl
  )
