install(PROGRAMS migrate_db_timeTracker migrate_db_memoryTracker db-migration.sh
  DESTINATION "${product}/${version}/tools/migration")
