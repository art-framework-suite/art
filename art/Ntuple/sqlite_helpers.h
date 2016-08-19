#ifndef art_Ntuple_sqlite_helpers_h
#define art_Ntuple_sqlite_helpers_h

// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "sqlite3.h"
#include "art/Ntuple/sqlite_column.h"
#include "art/Ntuple/sqlite_result.h"
#include "art/Ntuple/sqlite_stringstream.h"
#include "art/Ntuple/sqlite_query_impl.h"
#include "canvas/Utilities/Exception.h"

using namespace std::string_literals;

namespace sqlite {

  namespace detail {

    template <typename T>
    std::string column_info(column<T> const& col)
    {
      return col.name() + col.sqlite_type();
    }

    inline std::string columns() { return ""; }

    template <typename H, typename... T>
    std::string columns(H const& h, T const&... t)
    {
      return (sizeof...(T) != 0u) ? column_info(h) + "," + columns(t...) : column_info(h);
    }

    template <typename COL_PACK, std::size_t... I>
    std::string createTable_ddl(std::string const& tname,
                                COL_PACK const& cols,
                                std::index_sequence<I...>)
    {
      std::string ddl {"CREATE TABLE "s + tname + " ( "};
      ddl += columns(std::get<I>(cols)...);
      ddl += " )";
      return ddl;
    }

    bool hasTable (sqlite3* db, std::string const& name, std::string const& sqlddl);

  } // namespace detail

  //=====================================================================
  // General db functions

  sqlite3* openDatabaseFile(std::string const& filename);
  void     deleteTable(sqlite3* db, std::string const& tname);
  void     dropTable  (sqlite3* db, std::string const& tname);
  void     exec       (sqlite3* db, std::string const& ddl);

  unsigned nrows (sqlite3* db, std::string const& tname);

  template <typename... ARGS>
  void createTableIfNeeded(sqlite3* db,
                           sqlite3_int64& rowid,
                           std::string const& tname,
                           column_pack<ARGS...> const& cols,
                           bool const delete_contents)
  {
    std::string const sqlddl = detail::createTable_ddl(tname, cols, std::index_sequence_for<ARGS...>());
    if (!detail::hasTable(db, tname, sqlddl)) {
      exec(db, sqlddl);
    }
    else {
      if (delete_contents) {
        deleteTable(db, tname);
      }
      rowid = nrows(db, tname);
    }
  }

  //=======================================================================
  // Statistics helpers

  template<typename T = double>
  T min(sqlite3* db, std::string const& tname, std::string const& colname)
  {
    T t {};
    auto r = query(db, "select min("s+colname+") from " + tname);
    throw_if_empty(r) >> t;
    return t;
  }

  template<typename T = double>
  T max(sqlite3* db, std::string const& tname, std::string const& colname)
  {
    T t {};
    auto r = query(db, "select max("s+colname+") from " + tname);
    throw_if_empty(r) >> t;
    return t;
  }

  double mean  (sqlite3* db, std::string const& tname, std::string const& colname);
  double median(sqlite3* db, std::string const& tname, std::string const& colname);
  double rms   (sqlite3* db, std::string const& tname, std::string const& colname);

} //namespace sqlite

#endif /* art_Ntuple_sqlite_helpers_h */

// Local Variables:
// mode: c++
// End:
