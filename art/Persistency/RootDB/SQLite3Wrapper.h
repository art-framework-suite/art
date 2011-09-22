#ifndef art_Persistency_RootDB_SQLite3Wrapper_h
#define art_Persistency_RootDB_SQLite3Wrapper_h

// Sentry-like entity to manage the lifetime of an SQL database handle.

#include "boost/noncopyable.hpp"

#include <string>

extern "C" {
#include "sqlite3.h"
}

namespace art {
  class SQLite3Wrapper;
}

#include "TFile.h"

class art::SQLite3Wrapper : boost::noncopyable {
public:
  typedef int (*callback_t)(void *, int, char **, char **) ;

  SQLite3Wrapper();

  explicit
  SQLite3Wrapper(std::string const & key,
                 int flags = SQLITE_OPEN_READONLY);

  SQLite3Wrapper(TFile * tfile,
                 std::string const & key,
                 int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_TRANSIENT_DB);

  std::string const & key() const { return key_; }

  operator sqlite3 * () { return db_; }

  static bool wantTracing();

  void reset();

  void reset(std::string const & key,
             int flags = SQLITE_OPEN_READONLY);

  void reset(TFile * tfile,
             std::string const & key,
             int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_TRANSIENT_DB);

  ~SQLite3Wrapper();

  void swap(SQLite3Wrapper & other);

private:
  void initDB(int flags, TFile * tfile = 0);

  void maybeTrace() const;

  sqlite3 * db_;
  std::string key_;
};
#endif /* art_Persistency_RootDB_SQLite3Wrapper_h */

// Local Variables:
// mode: c++
// End:
