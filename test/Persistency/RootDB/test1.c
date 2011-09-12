#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int myvfs_init(void);

static int callback(void* junk, int cnt, char** vals, char** col_name)
{
   printf("\n");
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
   if (argc != 3) {
      fprintf(stderr, "usage: %s: <db-filename> <sql-statement>\n", argv[0]);
      exit(1);
   }
   myvfs_init();
   err = sqlite3_open_v2(argv[1], &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "myvfs");
   if (err) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
   }
   err = sqlite3_exec(db, argv[2], callback, 0, &error_msg);
   if (err != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", error_msg);
      sqlite3_free(error_msg);
   }
   sqlite3_close(db);
   return 0;
}

