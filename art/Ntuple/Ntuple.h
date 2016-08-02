#ifndef art_Ntuple_Ntuple_h
#define art_Ntuple_Ntuple_h

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "sqlite3.h"
#include "art/Ntuple/sqlite_helpers.h"
#include "art/Ntuple/Transaction.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace ntuple
{
  template <class ... ARGS> class Ntuple;
  template <size_t N> using name_array = std::array<std::string, N>;


  /// class Ntuple

  template <class ... ARGS>
  class Ntuple {
  public:
    using row_t = std::tuple<ARGS...>;
    static constexpr auto SIZE = std::tuple_size<row_t>::value;

    Ntuple(sqlite3* db,
           std::string const& name,
           name_array<SIZE> const& columns,
           bool const overwriteContents = false,
           std::size_t bufsize = 1000UL);

    Ntuple(std::string const& filename,
           std::string const& tablename,
           name_array<SIZE> const& columns,
           bool const overwriteContents = false,
           std::size_t bufsiz = 1000UL);

    ~Ntuple() noexcept;

    void insert(ARGS const...);
    int flush_no_throw();
    void flush();
    sqlite3_int64 lastRowid() const { return last_rowid_; }

  private:
    sqlite3*           db_;
    std::size_t        max_;
    std::vector<row_t> buffer_ {};
    sqlite3_stmt*      insert_statement_ {nullptr};
    sqlite3_int64      last_rowid_ {};
  };

}

template <class ...ARGS>
ntuple::Ntuple<ARGS...>::Ntuple(sqlite3* db,
                                std::string const& name,
                                name_array<SIZE> const& cnames,
                                bool const overwriteContents,
                                std::size_t bufsize) :
  db_{db},
  max_{bufsize}
{
  if (!db)
    throw art::Exception{art::errors::SQLExecutionError,"Attempt to create Ntuple with null database pointer"};

  sqlite::createTableIfNeeded<ARGS...>(db, last_rowid_, name, begin(cnames), end(cnames), overwriteContents );
  std::string sql {"INSERT INTO "};
  sql += name;
  sql += " VALUES (?";
  for (std::size_t i = 1; i < SIZE; ++i) { sql += ",?"; }
  sql += ")";
  int const rc = sqlite3_prepare_v2(db_,
                                    sql.c_str(),
                                    sql.size(),
                                    &insert_statement_,
                                    nullptr);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError,"Failed to prepare insertion statment"};

  buffer_.reserve(bufsize);
}

template <class ...ARGS>
ntuple::Ntuple<ARGS...>::Ntuple(std::string const& filename,
                                std::string const& name,
                                name_array<SIZE> const& cnames,
                                bool const overwriteContents,
                                std::size_t bufsize) :
  Ntuple{sqlite::openDatabaseFile(filename), name, cnames, overwriteContents, bufsize}
{}

template <class ... ARGS>
ntuple::Ntuple<ARGS...>::~Ntuple() noexcept
{
  int const rc = flush_no_throw();
  if (rc != SQLITE_OK)
    mf::LogError("SQLite") << "SQLite step failure while flushing";
  sqlite3_finalize(insert_statement_);
}

template <class ...ARGS>
void
ntuple::Ntuple<ARGS...>::insert(ARGS const... args)
{
  if (buffer_.size() == max_) {
    flush();
  }
  buffer_.emplace_back(args...);
  ++last_rowid_;
}

inline
void bind_one_parameter(sqlite3_stmt* s, std::size_t const idx, double const v)
{
  int const rc = sqlite3_bind_double(s, idx, v);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind double " + std::to_string(rc)};
}

inline
void bind_one_parameter(sqlite3_stmt* s, std::size_t const idx, int const v)
{
  int const rc = sqlite3_bind_int(s, idx, v);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind int " + std::to_string(rc)};
}

inline
void bind_one_parameter(sqlite3_stmt* s, std::size_t const idx, std::uint32_t const v)
{
  int const rc = sqlite3_bind_int64(s, idx, v);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind int " + std::to_string(rc)};
}

inline
void bind_one_parameter(sqlite3_stmt* s, std::size_t const idx, sqlite_int64 const v)
{
  int const rc = sqlite3_bind_int64(s, idx, v);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind int64 " + std::to_string(rc)};
}

inline
void bind_one_parameter(sqlite3_stmt* s, std::size_t const idx, std::string const& v)
{
  int const rc = sqlite3_bind_text(s, idx, v.c_str(), v.size(), nullptr);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind text " + std::to_string(rc)};
}

template <class TUP, size_t N>
struct bind_parameters
{
  static void bind(sqlite3_stmt* s, TUP const& t)
  {
    bind_parameters<TUP, N-1>::bind(s, t);
    bind_one_parameter(s, N, std::get<N-1>(t));
  }
};

template <class TUP>
struct bind_parameters<TUP, 1>
{
  static void bind(sqlite3_stmt* s, TUP const& t)
  {
    bind_one_parameter(s, 1, std::get<0>(t));
  }
};

template <class ...ARGS>
int
ntuple::Ntuple<ARGS...>::flush_no_throw()
{
  sqlite::Transaction txn {db_};
  for (auto const& r : buffer_) {
    bind_parameters<std::tuple<ARGS...>, SIZE>::bind(insert_statement_, r);
    int const rc = sqlite3_step(insert_statement_);
    if (rc != SQLITE_DONE)
      return rc;
    last_rowid_ = sqlite3_last_insert_rowid(db_);
    sqlite3_reset(insert_statement_);
  }
  txn.commit();
  buffer_.clear();
  return SQLITE_OK;
}

template <class ...ARGS>
void
ntuple::Ntuple<ARGS...>::flush()
{
  if (flush_no_throw() != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "SQLite step failure while flushing"};
}

#endif /* art_Ntuple_Ntuple_h */

// Local Variables:
// mode: c++
// End:
