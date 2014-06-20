#include "art/Framework/IO/Root/detail/readParameterSetsFromDB.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/make_ParameterSet.h"

#include "sqlite3.h"

namespace {
  // Recursively process all parameter sets in dependency order.
  void process_rows(sqlite3_stmt * curStmt,
                    sqlite3_stmt * depStmt,
                    art::FileFormatVersion const & ffv)
  {
    fhicl::ParameterSet pset;
    fhicl::make_ParameterSet(reinterpret_cast<char const *>(sqlite3_column_text(curStmt, 0)),
                             pset);
    if (ffv.value_ > 5) {
      std::vector<std::string> pset_keys(pset.get_pset_keys());
      for (auto const & key : pset_keys) {
        auto depIDString = pset.get<fhicl::ParameterSetID>(key).to_string();
        sqlite3_reset(depStmt);
        sqlite3_bind_text(depStmt, 1, depIDString.c_str(),
                          depIDString.size() + 1, SQLITE_STATIC);
        sqlite3_step(depStmt);
        process_rows(depStmt, depStmt, ffv);
      }
    }
    fhicl::ParameterSetRegistry::put(pset);
  }
}

void
art::detail::
readParameterSetsFromDB(SQLite3Wrapper & sqliteDB,
                        FileFormatVersion const & ffv)
{
  std::string extra_sql;
  if (ffv.value_ == 5) {
    extra_sql = " WHERE PSetBlob LIKE '%process_name:%'";
  }
  // Read the ParameterSets into memory.
  sqlite3_stmt *stmt = 0, *depStmt = 0;
  std::ostringstream initQuery;
  initQuery << "SELECT PSetBlob from ParameterSets"
            << extra_sql << ";";
  sqlite3_prepare_v2(sqliteDB, initQuery.str().c_str(),
                     -1, &stmt, NULL);
  sqlite3_prepare_v2(sqliteDB, "SELECT PSetBlob FROM ParameterSets WHERE ID = ?;",
                     -1, &depStmt, NULL);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    process_rows(stmt, depStmt, ffv);
  }
  sqlite3_finalize(stmt);
  sqlite3_finalize(depStmt);
}
