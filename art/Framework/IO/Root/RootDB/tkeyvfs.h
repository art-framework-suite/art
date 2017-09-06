#ifndef art_Persistency_RootDB_tkeyvfs_h
#define art_Persistency_RootDB_tkeyvfs_h

/*
 * Save db to root file on close.
*/

#ifndef TKEYVFS_NO_ROOT
#include "TFile.h"
#endif // TKEYVFS_NO_ROOT

#include <sqlite3.h>

extern "C" {
  int tkeyvfs_init(void);
  int tkeyvfs_open_v2(const char * filename, sqlite3 ** ppDb, int flags
#ifndef TKEYVFS_NO_ROOT
                      , TFile * rootFile
#endif // TKEYVFS_NO_ROOT
                     );
}

#endif /* art_Persistency_RootDB_tkeyvfs_h */

// Local Variables:
// mode: c++
// End:
