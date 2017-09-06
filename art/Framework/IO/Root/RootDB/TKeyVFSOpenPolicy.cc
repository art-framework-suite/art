#include "art/Framework/IO/Root/RootDB/TKeyVFSOpenPolicy.h"
#include "art/Framework/IO/Root/RootDB/tkeyvfs.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/sqlite/helpers.h"

art::TKeyVFSOpenPolicy::TKeyVFSOpenPolicy(TFile* const tfile, int const flags)
  : tfile_{tfile}
  , flags_{flags|SQLITE_OPEN_URI}
{}

sqlite3*
art::TKeyVFSOpenPolicy::open(std::string const& key)
{
  if (!key.size()) {
    throw Exception{errors::FileOpenError}
    << "Failed to open TKEYVFS DB due to empty key spec.\n";
  }

  auto const uriKey = cet::sqlite::assembleNoLockURI(key);
  sqlite3* db {nullptr};
  int const rc {tkeyvfs_open_v2(uriKey.c_str(), &db, flags_, tfile_)};
  if (rc != SQLITE_OK) {
    throw Exception{errors::FileOpenError}
      << "Failed to open requested DB, \""
      << key
      << "\" of type, \""
      << "tkeyvfs"
      << "\" -- "
      << sqlite3_errmsg(db)
      << '\n';
  }
  return db;
}
