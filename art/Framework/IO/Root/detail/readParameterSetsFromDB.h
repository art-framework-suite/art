#ifndef art_Framework_IO_Root_detail_readParameterSetsFromDB_h
#define art_Framework_IO_Root_detail_readParameterSetsFromDB_h

#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/RootDB/SQLite3Wrapper.h"

namespace art {
  namespace detail {
    void readParameterSetsFromDB(SQLite3Wrapper & sqliteDB,
                                 FileFormatVersion const & ffv);
  }
}

#endif /* art_Framework_IO_Root_detail_readParameterSetsFromDB_h */

// Local Variables:
// mode: c++
// End:
