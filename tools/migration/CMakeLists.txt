install(PROGRAMS art-2.11-migration.pl migrate_db_timeTracker migrate_db_memoryTracker db-migration.sh
  DESTINATION "${product}/${version}/tools/migration")
