#include "art/Persistency/RootDB/trace_sqldb.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

namespace {
  void traceit(void *, const char * zSQL)
  {
    mf::LogAbsolute("SQLTrace")
        << "SQLTrace: "
        << zSQL;
  }
}

void
art::trace_sqldb(sqlite3 * db, bool onOff, std::string const & dbName)
{
  // FIXME: Need to think of a way to store the DBName safely.
  if (onOff) {
    sqlite3_trace(db, traceit, 0);
  }
  else {
    sqlite3_trace(db, 0, 0);
  }
}
