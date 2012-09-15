#ifndef art_Persistency_RootDB_MetaDataAccess_h
#define art_Persistency_RootDB_MetaDataAccess_h
// Provide access to the One True meta data database handle.

#include "art/Persistency/RootDB/SQLite3Wrapper.h"

namespace art {
  class MetaDataAccess;
}

class art::MetaDataAccess {
public:
  MetaDataAccess(MetaDataAccess const&) = delete;
  MetaDataAccess& operator=(MetaDataAccess const&) = delete;

  static MetaDataAccess & instance();

  SQLite3Wrapper const & dbHandle() const { return dbHandle_; };
  SQLite3Wrapper & dbHandle() { return dbHandle_; };

private:
  MetaDataAccess();
  ~MetaDataAccess();

  SQLite3Wrapper dbHandle_;
};

#endif /* art_Persistency_RootDB_MetaDataAccess_h */

// Local Variables:
// mode: c++
// End:
