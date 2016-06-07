#ifndef art_Ntuple_sqlite_DBmanager_h
#define art_Ntuple_sqlite_DBmanager_h

#include "sqlite_helpers.h"

namespace sqlite
{

  class DBmanager {
  public:

    DBmanager(std::string const& filename)
      : db_{openDatabaseFile(filename)}
      , log_{!filename.empty()}
    {}

    ~DBmanager() {
      if (db_ != nullptr) sqlite3_close(db_);
    }

    bool logToDb() const { return log_; }

    sqlite3& operator* (){ return *db_; }
    sqlite3* operator->(){ return  db_; }
    sqlite3* get() const { return  db_; }

    operator sqlite3*() { return db_; }

  private:

    sqlite3* db_;
    bool log_;
  };

} //namespace sqlite
#endif /* art_Ntuple_sqlite_DBmanager_h */

// Local Variables:
// mode: c++
// End:
