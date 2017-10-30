#ifndef art_Framework_IO_Root_RootDB_TKeyVFSOpenPolicy_h
#define art_Framework_IO_Root_RootDB_TKeyVFSOpenPolicy_h

#include "TFile.h"
#include "sqlite3.h"

#include <string>

namespace art {

  class TKeyVFSOpenPolicy {
  public:
    explicit TKeyVFSOpenPolicy(TFile* const tfile,
                               int const flags = SQLITE_OPEN_READONLY);

    sqlite3* open(std::string const& key);

  private:
    TFile* tfile_;
    int flags_;
  };
}

#endif /* art_Framework_IO_Root_RootDB_TKeyVFSOpenPolicy_h */

// Local Variables:
// mode: c++
// End:
