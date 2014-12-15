#ifndef art_Ntuple_sqlite_helpers_h
#define art_Ntuple_sqlite_helpers_h

// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include "sqlite3.h"

using namespace std::string_literals;

namespace sqlite
{

  namespace detail
  {
    /// coltype<T> returns the string used to name the column type used
    /// to store an object of type T. There is no implementation of the
    /// general case; the template must be specialized for each
    /// supported type.
    template <class T> std::string coltype();

    /// Function template createTable_ddl returns the SQL DDL 'CREATE
    /// TABLE' statement to declare a table named 'tname', suitable
    /// for storing tuples of type TUP, and with column names
    /// determined by the range [beginCol, endCol).
    template <class TUP, class IT>
    std::string createTable_ddl(std::string const& tname, IT beginCol, IT endCol);

    /// struct BuildSQL a helper used in creatTable_ddl.
    template <class TUP, std::size_t I> struct BuildSQL;

    // Only types for which a coltype<> specialization is implemented
    // are supported.
    template <> inline std::string coltype<double>()      { return " numeric"; }
    template <> inline std::string coltype<float>()       { return " numeric"; }

    template <> inline std::string coltype<int>()         { return " integer"; }
    template <> inline std::string coltype<long>()        { return " integer"; }
    template <> inline std::string coltype<long long>()   { return " integer"; }

    template <> inline std::string coltype<unsigned int>()         { return " integer"; }
    template <> inline std::string coltype<unsigned long>()        { return " integer"; }
    template <> inline std::string coltype<unsigned long long>()   { return " integer"; }

    template <> inline std::string coltype<std::string>() { return " text"; }

    //=====================================================================
    // BEGIN IMPLEMENTATION FOR NAMESPACE deta
    // BuildSQL is a helper struct, with function addMore. A struct is
    // needed because partial specialization of function templates is
    // not supported in C++11.
    template <class TUP, std::size_t I>
    struct BuildSQL
    {
      static constexpr std::size_t SIZE = std::tuple_size<TUP>::value;
      using result_t = typename std::tuple_element<I, TUP>::type;

      template <class IT>
      static void addMore(std::string& cmd,
                          IT beginCol, IT endCol)
      {
        BuildSQL<TUP,I-1>::addMore(cmd, beginCol, endCol);
        cmd += ", ";
        cmd += *(beginCol+I);
        cmd += " ";
        cmd += coltype<result_t>();
      }
    };

    template <class TUP>
    struct BuildSQL<TUP,0>
    {
      static constexpr std::size_t SIZE = std::tuple_size<TUP>::value;
      using result_t = typename std::tuple_element<0, TUP>::type;

      template <class IT>
      static void addMore(std::string& cmd,
                          IT beginCol, IT /* unused */)
      {
        cmd += *beginCol;
        cmd += " ";
        cmd += coltype<result_t>();
      }
    };

    //==========================================================================
    template <class TUP, class IT>
    std::string createTable_ddl(std::string const& tname,
                                IT beginCol,
                                IT endCol)
    {
      std::string ddl("CREATE TABLE "s + tname + " ( "s);
      BuildSQL<TUP, std::tuple_size<TUP>::value-1>::addMore(ddl, beginCol, endCol);
      ddl += " )";
      return ddl;
    }

    //=======================================================================
    struct query_result
    {
      std::vector<std::string> ddl;
      int count = 0;
    };

    void         exec (sqlite3* db, std::string const& ddl);
    query_result query( sqlite3* db, std::string const& ddl );

    int  getResult(void* data, int ncols [[gnu::unused]], char** results, char** cnames);
    bool hasTable (sqlite3* db, std::string const& name, std::string const& sqlddl);

  } // namespace detail

  //=====================================================================
  // General db functions

  sqlite3* openDatabaseFile(std::string const& filename);
  void     deleteTable( sqlite3* db, std::string const& tname );
  void     dropTable  ( sqlite3* db, std::string const& tname );

  //=====================================================================
  // Conversion functions for querying results

  template<typename T = std::vector<std::string> >
  T convertTo( const std::vector<std::string>& ddl ) {
    return ddl;
  }

  template<>
  inline double convertTo<double>( const std::vector<std::string>& ddl ) {
    return std::stod( ddl[0] );
  }

  template<>
  inline uint32_t convertTo<uint32_t>( const std::vector<std::string>& ddl) {
    return std::stoul( ddl[0] );
  }

  //=====================================================================
  // Querying facilities

  template<typename T>
  T query_db(sqlite3* db, std::string const& ddl )
  {
    detail::query_result const res = detail::query( db, ddl );
    if (res.count == 0) throw std::runtime_error("sqlite query_db unsuccessful");
    return convertTo<T>( res.ddl );
  }

  template<typename T>
  auto getUniqueEntries( sqlite3* db, const std::string& tname, const std::string& colname ) {
    return query_db<std::vector<T>>( db, "select distinct "s+colname+" from "s+tname );
  }

  //=====================================================================
  template <class ...ARGS, class IT>
  void createTableIfNeeded(sqlite3* db,
                           sqlite3_int64& rowid,
                           std::string const& tname,
                           IT beginCol,
                           IT endCol,
                           bool const delete_contents )
  {
    std::string const sqlddl = detail::createTable_ddl<std::tuple<ARGS...>>(tname, beginCol, endCol);
    if (!detail::hasTable(db, tname, sqlddl)) {
      detail::exec(db, sqlddl);
    }
    else {

      if (delete_contents) {
        deleteTable( db, tname );
      }

      rowid = query_db<uint32_t>( db, "select count(*) from "s + tname );

    }
  }

  //=======================================================================
  // Statistics helpers

  template<typename T = double>
  T min( sqlite3* db, const std::string& tname, const std::string& colname ) {
    return query_db<T>( db, "select min("s+colname+") from "s + tname );
  }

  template<typename T = double>
  T max( sqlite3* db, const std::string& tname, const std::string& colname ) {
    return query_db<T>( db, "select max("s+colname+") from "s + tname );
  }

  double mean  ( sqlite3* db, std::string const& tname, std::string const& colname );
  double median( sqlite3* db, std::string const& tname, std::string const& colname );
  double rms   ( sqlite3* db, std::string const& tname, std::string const& colname );

} //namespace sqlite
#endif /* art_Ntuple_sqlite_helpers_h */

// Local Variables:
// mode: c++
// End:
