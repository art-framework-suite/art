#define _GNU_SOURCE
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int myvfs_init(void);

static int
callback(void* junk __attribute__((unused)),
         int cnt,
         char** vals,
         char** col_name)
{
  int i = 0;
  printf("\n");
  for (i = 0; i < cnt; ++i) {
    if (vals[i]) {
      printf("%s: %s\n", col_name[i], vals[i]);
    } else {
      printf("%s: %s\n", col_name[i], "NULL");
    }
  }
  return 0;
}

int
main(int argc, char** argv)
{
  sqlite3* db = 0;
  char* error_msg = 0;
  int err = 0;
  if (argc < 2 || argc > 3) {
    fprintf(stderr,
            "usage: %s: <db-filename> [<sql-statement>]. If <sql-statement> is "
            "omitted take from stdin.\n",
            argv[0]);
    exit(1);
  }
  myvfs_init();
  err = sqlite3_open_v2(
    argv[1], &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "myvfs");
  if (err) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  if (argc == 3) {
    err = sqlite3_exec(db, argv[2], callback, 0, &error_msg);
  } else {
    char* buf = 0;
    size_t n = 0;
    getline(&buf, &n, stdin);
    err = sqlite3_exec(db, buf, callback, 0, &error_msg);
    free(buf);
  }
  if (err != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", error_msg);
    sqlite3_free(error_msg);
  }
  sqlite3_close(db);
  return 0;
}
