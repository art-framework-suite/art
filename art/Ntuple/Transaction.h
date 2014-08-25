#ifndef art_Ntuple_Transaction_h
#define art_Ntuple_Transaction_h

struct sqlite3;

namespace sqlite
{

  /// A Transaction object encapsulates a single SQLite transaction. Such
  /// transactions can not be nested. Look into using SAVEPOINT if you
  /// need nesting.
  class Transaction
  {
  public:
    /// Creating a Transaction begins a transaction in the given db. It is
    /// expected that the database has already be opened.
    explicit Transaction(sqlite3* db);

    /// Transactions may not be copied or assigned.
    Transaction(Transaction const&) = delete;
    Transaction(Transaction &&) = delete;
    Transaction& operator=(Transaction const&) = delete;
    Transaction& operator=(Transaction &&) = delete;

    /// Destroying the Transaction will roll back the associated
    /// transaction, unless 'commit' has been called.
    ~Transaction();

    /// Commit the associated SQLite transaction. Unless committed, the
    /// transaction will be rolled back upon destruction. 'commit' may
    /// only be called once per Transaction.
    void commit();

  private:
    sqlite3* db_;
  };
}

#endif /* art_Ntuple_Transaction_h */

// Local Variables:
// mode: c++
// End:
