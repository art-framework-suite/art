#include "art/Ntuple/Transaction.h"

#include <stdexcept>
#include <assert.h>
#include "sqlite3.h"

sqlite::Transaction::Transaction(sqlite3* db) :
  db_(db)
{
  assert(db_);
  int rc = sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK)
    throw std::runtime_error("Failed to start SQLite transaction");
}

sqlite::Transaction::~Transaction()
{
  // We can't throw an exception from our destructor, so we just
  // swallow any error.
  if (db_) sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);        
}

void
sqlite::Transaction::commit()
{
  assert(db_);
  int rc = sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK)
    throw std::runtime_error("Failed to commit SQLite transaction");
  db_ = nullptr;
}





