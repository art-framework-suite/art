#include "art/Framework/IO/Root/detail/readParameterSetsFromDB.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/make_ParameterSet.h"

#include "sqlite3.h"

#include <iostream>

namespace {

#if 0 /* debug */
  size_t get_total_rows(art::SQLite3Wrapper & sqliteDB)
  {
    size_t result { 0 };
    sqlite3_stmt *stmt = 0;
    sqlite3_prepare_v2(sqliteDB, "SELECT COUNT (*) FROM ParameterSets;",
                       -1,  &stmt, NULL);
    sqlite3_step(stmt);
    result = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return result;
  }
#endif

  // Recursively process all parameter sets in dependency order.
  class RowProcessor {
public:
    RowProcessor(art::SQLite3Wrapper & sqliteDB,
                 art::FileFormatVersion const & ffv)
:
      process_((ffv.value_ == 5) ?
               &RowProcessor::process_v5_ :
               &RowProcessor::process_v6p_),
      reg_(fhicl::ParameterSetRegistry::get())
      {
        std::string extra_sql;
        if (ffv.value_ == 5) {
          extra_sql = " WHERE PSetBlob LIKE '%process_name:%'";
        }
        std::function<void (sqlite3_stmt *, sqlite3_stmt *)> process;
        // Read the ParameterSets into memory.
        sqlite3_stmt *stmt = 0, *depStmt = 0;
        std::ostringstream initQuery;
        initQuery << "SELECT PSetBlob, ID from ParameterSets"
                  << extra_sql << ";";
        sqlite3_prepare_v2(sqliteDB, initQuery.str().c_str(),
                           -1, &stmt, NULL);
        sqlite3_prepare_v2(sqliteDB, "SELECT PSetBlob FROM ParameterSets WHERE ID = ?;",
                           -1, &depStmt, NULL);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          fhicl::ParameterSetID id (reinterpret_cast<char const *>(sqlite3_column_text(stmt, 1)));
          if (reg_.find(id) != reg_.end()) {
            continue;
          }
          (this->*process_)(stmt, depStmt);
        }
        sqlite3_finalize(stmt);
        sqlite3_finalize(depStmt);
      }

private:
    void process_v5_ (sqlite3_stmt * curStmt,
                     sqlite3_stmt *)
      {
        fhicl::ParameterSet pset;
        fhicl::make_ParameterSet(reinterpret_cast<char const *>(sqlite3_column_text(curStmt, 0)),
                                 pset);
        fhicl::ParameterSetRegistry::put(pset);
      }

    void process_v6p_ (sqlite3_stmt * curStmt,
                      sqlite3_stmt * depStmt)
      {
        fhicl::ParameterSet pset;
        fhicl::make_ParameterSet(reinterpret_cast<char const *>(sqlite3_column_text(curStmt, 0)),
                                 pset);
        std::vector<std::string> pset_keys(pset.get_pset_keys());
        for (auto const & key : pset_keys) {
          auto const & depID = pset.get<fhicl::ParameterSetID>(key);
          if (reg_.find(depID) != reg_.end()) {
            continue;
          }
          auto depIDString = depID.to_string();
          sqlite3_reset(depStmt);
          sqlite3_bind_text(depStmt, 1, depIDString.c_str(),
                            depIDString.size() + 1, SQLITE_STATIC);
          sqlite3_step(depStmt);
          process_v6p_(depStmt, depStmt);
        }
        fhicl::ParameterSetRegistry::put(pset);
      }

    void (RowProcessor::* process_) (sqlite3_stmt * curStmt,
                                     sqlite3_stmt * depStmt);

    fhicl::ParameterSetRegistry::collection_type const & reg_;
  };

}

void
art::detail::
readParameterSetsFromDB(SQLite3Wrapper & sqliteDB,
                        FileFormatVersion const & ffv)
{
  RowProcessor rp(sqliteDB, ffv);
}
