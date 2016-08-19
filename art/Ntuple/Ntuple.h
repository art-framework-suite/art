#ifndef art_Ntuple_Ntuple_h
#define art_Ntuple_Ntuple_h

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "sqlite3.h"
#include "art/Ntuple/sqlite_helpers.h"
#include "art/Ntuple/sqlite_insert_impl.h"
#include "art/Ntuple/Transaction.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace ntuple
{
  template <class ... ARGS> class Ntuple;

  template <class ... ARGS>
  class Ntuple {
  public:
    using row_t = std::tuple<std::unique_ptr<ARGS>...>;
    static constexpr auto SIZE = std::tuple_size<row_t>::value;

    template <size_t N>
    using name_array = sqlite::name_array<N>;

    Ntuple(sqlite3* db,
           std::string const& name,
           name_array<SIZE> const& columns,
           bool const overwriteContents = false,
           std::size_t bufsize = 1000ull);

    Ntuple(std::string const& filename,
           std::string const& tablename,
           name_array<SIZE> const& columns,
           bool const overwriteContents = false,
           std::size_t bufsiz = 1000ull);

    ~Ntuple() noexcept;

    void insert(ARGS const...);
    std::string const& name() const { return name_; }
    sqlite3* db() const { return db_; }
    int flush_no_throw();
    void flush();
    sqlite3_int64 lastRowid() const { return last_rowid_; }

  private:
    sqlite3*           db_;
    std::string        name_;
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
  name_{name},
  max_{bufsize}
{
  if (!db)
    throw art::Exception{art::errors::SQLExecutionError,"Attempt to create Ntuple with null database pointer"};

  static_assert(sizeof...(ARGS) == SIZE, "Number of column names must equal the number of template arguments.");

  sqlite::createTableIfNeeded(db,
                              last_rowid_,
                              name,
                              sqlite::make_column_pack<ARGS...>(cnames, std::index_sequence_for<ARGS...>()),
                              overwriteContents);

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
  buffer_.emplace_back(std::make_unique<ARGS>(args)...);
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

inline
void bind_one_null(sqlite3_stmt* s, std::size_t const idx)
{
  int const rc = sqlite3_bind_null(s, idx);
  if (rc != SQLITE_OK)
    throw art::Exception{art::errors::SQLExecutionError, "Failed to bind text " + std::to_string(rc)};
}

template <class TUP, size_t N>
struct bind_parameters
{
  static void bind(sqlite3_stmt* s, TUP const& t)
  {
    bind_parameters<TUP, N-1>::bind(s, t);
    if (auto& param = std::get<N-1>(t))
      bind_one_parameter(s, N, *param);
    else
      bind_one_null(s,N);
  }
};

template <class TUP>
struct bind_parameters<TUP, 1>
{
  static void bind(sqlite3_stmt* s, TUP const& t)
  {
    if (auto& param = std::get<0>(t))
      bind_one_parameter(s, 1, *param);
    else
      bind_one_null(s,1);
  }
};

template <class ...ARGS>
int
ntuple::Ntuple<ARGS...>::flush_no_throw()
{
  sqlite::Transaction txn {db_};
  for (auto const& r : buffer_) {
    bind_parameters<row_t, SIZE>::bind(insert_statement_, r);
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
