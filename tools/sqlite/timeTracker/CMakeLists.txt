install(FILES
  event-summary.sql
  max-events.sql
  module-summary.sql
  read-event-summary.sql
  DESTINATION "${${PROJECT_NAME}_TOOLS_DIR}/sqlite/timeTracker")
