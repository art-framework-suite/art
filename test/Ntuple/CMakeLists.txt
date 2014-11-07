
cet_test(ntuple_t
         LIBRARIES art_Ntuple ${SQLITE3}
         )

cet_test_env(CLEAR "FHICL_FILE_PATH=${CMAKE_CURRENT_SOURCE_DIR}")

cet_test( messagefacility_destination_sqlite_t HANDBUILT
  TEST_EXEC ELdestinationTester
  TEST_ARGS sqlite_mfplugin_t.fcl
  )
