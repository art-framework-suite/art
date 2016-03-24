#include "art/Framework/IO/Root/detail/getFileContributors.h"
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

std::vector<EventRange>
art::detail::getFileContributors()
{
  return {};
}

std::vector<EventRange>
art::detail::getRunContributors(TFile & file,
                                RunNumber_t const r)
{
  art::SQLite3Wrapper db {&file, "RootFileDB"};
  auto const& filename = file.GetName();

  sqlite3_stmt* stmt {nullptr};
  std::string const ddl {"SELECT * FROM EventRanges WHERE ROWID IN "
      "(SELECT EventRangesID FROM RangeSets_EventRanges WHERE RangeSetsID IN "
      "(SELECT rowid FROM RangeSets WHERE Run="+std::to_string(r)+"));"};
  auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
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

  return ranges;
}


std::vector<EventRange>
art::detail::getSubRunContributors(TFile & file,
                                   RunNumber_t const r,
                                   SubRunNumber_t const sr)
{
  art::SQLite3Wrapper db {&file, "RootFileDB"};
  return getSubRunContributors(db, file.GetName(), r, sr);
}

std::vector<EventRange>
art::detail::getSubRunContributors(sqlite3* db,
                                   std::string const& filename,
                                   RunNumber_t const r,
                                   SubRunNumber_t const sr)
{
  sqlite3_stmt* stmt {nullptr};
  std::string const ddl {"SELECT * FROM EventRanges WHERE ROWID IN "
      "(SELECT EventRangesID FROM RangeSets_EventRanges WHERE RangeSetsID IN "
      "(SELECT rowid FROM RangeSets WHERE Run="+std::to_string(r)+")) AND "
      "SubRun="+std::to_string(sr)+";"};
  auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
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

  return ranges;
}

// std::vector<EventRange>
// art::detail::getSubRunContributors(TFile& file,
//                                    unsigned const rangeSetID)
// {
//   art::SQLite3Wrapper db {&file, "RootFileDB"};
//   return getSubRunContributors(db, file.GetName(), r, sr);

//   sqlite3_stmt* stmt {nullptr};
//   std::string const ddl {"SELECT * FROM EventRanges WHERE ROWID IN "
//       "(SELECT EventRangesID FROM RangeSets_EventRanges WHERE RangeSetsID IN "
//       "(SELECT rowid FROM RangeSets WHERE Run="+std::to_string(r)+")) AND "
//       "SubRun="+std::to_string(sr)+";"};
//   auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
//   successful_prepare(rc, filename);

//   std::vector<EventRange> ranges;
//   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//     ranges.emplace_back(sqlite3_column_int(stmt,0),
//                         sqlite3_column_int(stmt,1),
//                         sqlite3_column_int(stmt,2));
//   }
//   successful_step(rc, sqlite3_errmsg(db), filename);

//   rc = sqlite3_finalize(stmt);
//   successful_finalize(rc, sqlite3_errmsg(db), filename);

//   return ranges;
// }


art::RangeSet
art::detail::getSubRunContributors(sqlite3* db,
                                   std::string const& filename,
                                   std::size_t const rangeSetID)
{
  sqlite3_stmt* stmt {nullptr};
  std::string const run_ddl {"SELECT Run FROM RangeSets WHERE rowid=="
      + std::to_string(rangeSetID) + ";"};
  auto rc = sqlite3_prepare_v2(db, run_ddl.c_str(), -1, &stmt, nullptr);
  successful_prepare(rc, filename, run_ddl);

  rc = sqlite3_step(stmt);
  auto const r = static_cast<RunNumber_t>(sqlite3_column_int(stmt,0));
  rc = sqlite3_finalize(stmt);
  successful_finalize(rc, sqlite3_errmsg(db), filename);

  std::string const ddl {"SELECT SubRun, begin, end FROM EventRanges WHERE rowid IN"
      "(SELECT EventRangesID FROM RangeSets_EventRanges WHERE RangeSetsID=="
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

  return RangeSet{r, ranges};
}
