#ifndef art_Persistency_RootDB_TKeyVFSOpenPolicy_h
#define art_Persistency_RootDB_TKeyVFSOpenPolicy_h

#include "art/Persistency/RootDB/tkeyvfs.h"
#include "canvas/Utilities/Exception.h"

#include "TFile.h"

#include "sqlite3.h"

namespace art {

  class TKeyVFSOpenPolicy {
  public:

    explicit TKeyVFSOpenPolicy(TFile* const tfile,
                               int const flags = SQLITE_OPEN_READONLY)
      : tfile_{tfile}
      , flags_{flags}
    {}

    sqlite3* open(std::string const& key)
    {
      if (!key.size()) {
        throw Exception{errors::FileOpenError}
        << "Failed to open TKEYVFS DB due to empty key spec.\n";
      }
      sqlite3* db {nullptr};
      int const rc {tkeyvfs_open_v2(key.c_str(), &db, flags_, tfile_)};
      if (rc != SQLITE_OK) {
        throw Exception{errors::FileOpenError}
        << "Failed to open requested DB, \""
             << key
             << "\" of type, \""
             << "tkeyvfs"
             << "\" -- "
             << sqlite3_errmsg(db)
             << "\n";
      }
      return db;
    }

  private:
    TFile* tfile_;
    int flags_;
  };
}

#endif

// Local variable:
// mode: c++
// End:
