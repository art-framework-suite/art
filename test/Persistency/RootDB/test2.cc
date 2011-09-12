#include "art/Persistency/RootDB/tkeyvfs.h"

extern "C" {
#include <sqlite3.h>
}

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef TKEYVFS_DO_ROOT
#include "TFile.h"
#endif

using namespace std;

static int callback(void* junk, int cnt, char** vals, char** col_name)
{
   int i = 0;
   for (i = 0; i < cnt; ++i) {
      if (vals[i]) {
         printf("%s: %s\n", col_name[i], vals[i]);
      }
      else {
         printf("%s: %s\n", col_name[i], "NULL");
      }
   }
   return 0;
}

int main(int argc, char** argv)
{
   sqlite3* db = 0;
   char* error_msg = 0;
   int err = 0;
   if (argc != 5) {
      fprintf(stderr, "usage: %s: <op> <db-filename> <db-tkeyname> <sql-statement>\n", argv[0]);
      exit(1);
   }
   tkeyvfs_init();
   if (!strcmp(argv[1], "r")) {
#ifdef TKEYVFS_DO_ROOT
      TFile* rootFile = new TFile(argv[2]);
#endif
      err = tkeyvfs_open_v2(argv[3], &db, SQLITE_OPEN_READONLY, "tkeyvfs"
#ifdef TKEYVFS_DO_ROOT
                            , rootFile
#endif
                           );
   }
   else {
#ifdef TKEYVFS_DO_ROOT
     TFile* rootFile = new TFile(argv[2], "RECREATE");
#endif
     err = tkeyvfs_open_v2(argv[3], &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "tkeyvfs"
#ifdef TKEYVFS_DO_ROOT
                           , rootFile
#endif
                          );
   }
   if (err) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
   }
   err = sqlite3_exec(db, argv[4], callback, 0, &error_msg);
   if (err != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", error_msg);
      sqlite3_free(error_msg);
   }
   sqlite3_close(db);
   return 0;
}

