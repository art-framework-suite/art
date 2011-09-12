#ifndef art_Persistency_RootDB_tkeyvfs_h
#define art_Persistency_RootDB_tkeyvfs_h

/*
 * Save db to root file on close.
*/

#if 0
#ifndef TKEYVFS_DO_ROOT
#define TKEYVFS_DO_ROOT 1
#endif // TKEYVFS_DO_ROOT
#endif // 0

#ifdef TKEYVFS_DO_ROOT
#include "TFile.h"
#endif // TKEYVFS_DO_ROOT

#include <sqlite3.h>

extern "C" {
  int tkeyvfs_init(void);
  int tkeyvfs_open_v2(const char * filename, sqlite3 ** ppDb, int flags, const char * zVfs
#ifdef TKEYVFS_DO_ROOT
                      ,TFile * rootFile
#endif // TKEYVFS_DO_ROOT
                     );
}

#endif /* art_Persistency_RootDB_tkeyvfs_h */

// Local Variables:
// mode: c++
// End:
