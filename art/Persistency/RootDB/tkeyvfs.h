#ifndef TKEYVFS_H
#define TKEYVFS_H

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

int tkeyvfs_init(void);
int tkeyvfs_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs
#ifdef TKEYVFS_DO_ROOT
, TFile* rootFile
#endif // TKEYVFS_DO_ROOT
);

#endif // TKEYVFS_H
