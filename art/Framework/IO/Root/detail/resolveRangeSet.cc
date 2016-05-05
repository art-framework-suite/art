#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"

namespace {

  void successful_prepare(int const rc,
                          std::string const& filename,
                          std::string const& statement)
  {
    if (rc == SQLITE_OK) return;
    throw art::Exception{art::errors::SQLExecutionError}
      << "Error in preparing statement for getting contributors.\n"
      << "File: " << filename << '\n'
      << "Preparation statement: " << statement << '\n';
  }

  void successful_step(int const rc,
                       std::string const& sqlErrMsg,
                       std::string const& filename)
  {
    if (rc == SQLITE_DONE) return;
    throw art::Exception{art::errors::SQLExecutionError}
      << "Unexpected status from table read: "
      << sqlErrMsg
      << " (" << rc <<").\n"
      << "File: " << filename << '\n';
  }

  void successful_finalize(int const rc,
                           std::string const& sqlErrMsg,
                           std::string const& filename)
  {
    if (rc == SQLITE_OK) return;
    throw art::Exception{art::errors::SQLExecutionError}
      << "Unexpected status from DB status cleanup: "
      << sqlErrMsg
      << " (" << rc <<").\n"
      << "File: " << filename << '\n';
  }


}

using art::EventRange;

art::RangeSet
art::detail::resolveRangeSet(sqlite3* db,
                             std::string const& filename,
                             BranchType const bt,
                             unsigned const rangeSetID)
{
  // Invalid rangeSetID check
  if (rangeSetID == std::numeric_limits<unsigned>::max())
    return RangeSet::invalid();

  sqlite3_stmt* stmt {nullptr};
  std::string const run_ddl {"SELECT Run FROM "+BranchTypeToString(bt)+"RangeSets WHERE rowid=="
      + std::to_string(rangeSetID) + ";"};
  auto rc = sqlite3_prepare_v2(db, run_ddl.c_str(), -1, &stmt, nullptr);
  successful_prepare(rc, filename, run_ddl);

  rc = sqlite3_step(stmt);
  auto const r = static_cast<RunNumber_t>(sqlite3_column_int(stmt,0));
  rc = sqlite3_finalize(stmt);
  successful_finalize(rc, sqlite3_errmsg(db), filename);

  std::string const ddl {"SELECT SubRun, begin, end FROM EventRanges WHERE rowid IN"
      "(SELECT EventRangesID FROM "+BranchTypeToString(bt)+"RangeSets_EventRanges WHERE RangeSetsID=="
      + std::to_string(rangeSetID) + ");"};
  rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
  successful_prepare(rc, filename, ddl);

  std::vector<EventRange> ranges;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    ranges.emplace_back(sqlite3_column_int(stmt,0),
                        sqlite3_column_int(stmt,1),
                        sqlite3_column_int(stmt,2));
  }
  successful_step(rc, sqlite3_errmsg(db), filename);

  rc = sqlite3_finalize(stmt);
  successful_finalize(rc, sqlite3_errmsg(db), filename);

  return ranges.empty() ? RangeSet::forRun(RunID{r}) : RangeSet{r, ranges};
}
