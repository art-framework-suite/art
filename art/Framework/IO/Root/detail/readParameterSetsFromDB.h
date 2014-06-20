#ifndef HYHYHSDAGSDFA
#define HYHYHSDAGSDFA

#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"

namespace art {
  namespace detail {
    void readParameterSetsFromDB(SQLite3Wrapper & sqliteDB,
                                 FileFormatVersion const & ffv);
  }
}

#endif
