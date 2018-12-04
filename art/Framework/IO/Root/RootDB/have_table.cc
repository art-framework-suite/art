#include "art/Framework/IO/Root/RootDB/have_table.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/compiler_macros.h"

bool
art::have_table(sqlite3* db,
                std::string const& table,
                std::string const& filename)
{
  bool result = false;
  sqlite3_stmt* stmt = nullptr;
  std::string const ddl{
    "select 1 from sqlite_master where type='table' and name='" + table + "';"};
  auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    switch (rc = sqlite3_step(stmt)) {
      case SQLITE_ROW:
        result = true; // Found the table.
        FALLTHROUGH;
      case SQLITE_DONE:
        rc = SQLITE_OK; // No such table.
        break;
      default:
        break;
    }
  }
  rc = sqlite3_finalize(stmt);
  if (rc != SQLITE_OK) {
    throw art::Exception(art::errors::FileReadError)
      << "Error interrogating SQLite3 DB in file " << filename << ".\n";
  }
  return result;
}
