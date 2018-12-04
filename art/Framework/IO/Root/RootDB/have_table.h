#ifndef art_Framework_IO_Root_RootDB_have_table_h
#define art_Framework_IO_Root_RootDB_have_table_h

extern "C" {
#include "sqlite3.h"
}

#include <string>

namespace art {
  bool have_table(sqlite3* db,
                  std::string const& table,
                  std::string const& filename);
}

#endif /* art_Framework_IO_Root_RootDB_have_table_h */

// Local Variables:
// mode: c++
// End:
