#ifndef art_Persistency_RootDB_trace_sqldb_h
#define art_Persistency_RootDB_trace_sqldb_h

extern "C" {
#include "sqlite3.h"
}

#include <string>

namespace art {
  void trace_sqldb(sqlite3 * db, bool onOff, std::string const & dbName = "UNKNOWN");
}
#endif /* art_Persistency_RootDB_trace_sqldb_h */

// Local Variables:
// mode: c++
// End:
