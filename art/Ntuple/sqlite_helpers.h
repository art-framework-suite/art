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
#include "art/Ntuple/sqlite_stringstream.h"
#include "art/Utilities/Exception.h"

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
    // BEGIN IMPLEMENTATION FOR NAMESPACE detail
    // BuildSQL is a helper struct, with function addMore. A struct is
    // needed because partial specialization of function templates is
    // not supported in C++11.
    template <class TUP, std::size_t I>
    struct BuildSQL
    {
      static constexpr std::size_t SIZE = std::tuple_size<TUP>::value;
      using result_t = std::tuple_element_t<I, TUP>;

      template <class IT>
      static void addMore(std::string& cmd, IT beginCol, IT endCol)
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
      using result_t = std::tuple_element_t<0, TUP>;

      template <class IT>
      static void addMore(std::string& cmd, IT beginCol, IT /* unused */)
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
      std::string ddl {"CREATE TABLE "s + tname + " ( "};
      BuildSQL<TUP, std::tuple_size<TUP>::value-1>::addMore(ddl, beginCol, endCol);
      ddl += " )";
      return ddl;
    }

    //=======================================================================
    struct query_result {
      std::vector<sqlite::stringstream> data;
    };

    query_result query( sqlite3* db, std::string const& ddl );

    int  getResult(void* data, int ncols [[gnu::unused]], char** results, char** cnames);
    bool hasTable (sqlite3* db, std::string const& name, std::string const& sqlddl);

  } // namespace detail

  //=====================================================================
  // General db functions

  sqlite3* openDatabaseFile(std::string const& filename);
  void     deleteTable( sqlite3* db, std::string const& tname );
  void     dropTable  ( sqlite3* db, std::string const& tname );
  void     exec       ( sqlite3* db, std::string const& ddl   );

  //=====================================================================
  // Conversion functions for querying results

  template < typename T >
  inline T convertTo( std::vector<sqlite::stringstream>& )
  {
    return T();
  }

  template <>
  inline double convertTo<double>( std::vector<sqlite::stringstream>& data )
  {
    if ( data.size() != 1 || data[0].size() != 1 ) {
      throw art::Exception(art::errors::LogicError) << "SQLite results are not unique";
    }
    return std::stod( data[0][0] );
  }

  template<>
  inline uint32_t convertTo<uint32_t>( std::vector<sqlite::stringstream>& data )
  {
    if ( data.size() != 1 || data[0].size() != 1 ) {
      throw art::Exception(art::errors::LogicError) << "SQLite results are not unique";
    }
    return std::stoul( data[0][0] );
  }

  template<>
  inline
  std::vector<std::string>
  convertTo<std::vector<std::string> >( std::vector<sqlite::stringstream> & data )
  {
    std::vector<std::string> strList;
    for ( auto & entry : data ) {
      std::string tmpstr;
      while ( !entry.empty() ) {
        std::string token;
        entry >> token;
        tmpstr += token;
      }
      strList.push_back( tmpstr );
    }
    return strList;
  }

  //=====================================================================
  // Querying facilities

  inline std::vector<sqlite::stringstream> query_db(sqlite3* db, std::string const& ddl, bool const do_throw = true)
  {
    detail::query_result res = detail::query( db, ddl );
    if ( res.data.empty() && do_throw ) throw art::Exception(art::errors::SQLExecutionError) << "SQLite query_db unsuccessful";
    return std::move(res.data);
  }

  template<typename T>
  decltype(auto) query_db(sqlite3* db, std::string const& ddl, bool const do_throw = true)
  {
    detail::query_result res = detail::query( db, ddl );
    if ( res.data.empty() && do_throw ) throw art::Exception(art::errors::SQLExecutionError) << "SQLite query_db unsuccessful";
    return convertTo<T>( res.data );
  }

  template<typename T>
  auto getUniqueEntries( sqlite3* db, const std::string& tname, const std::string& colname )
  {
    return query_db<std::vector<T>>( db, "select distinct "s+colname+" from "+tname );
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
      exec(db, sqlddl);
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
  T min( sqlite3* db, const std::string& tname, const std::string& colname )
  {
    return query_db<T>( db, "select min("s+colname+") from " + tname );
  }

  template<typename T = double>
  T max( sqlite3* db, const std::string& tname, const std::string& colname )
  {
    return query_db<T>( db, "select max("s+colname+") from " + tname );
  }

  double mean  ( sqlite3* db, std::string const& tname, std::string const& colname );
  double median( sqlite3* db, std::string const& tname, std::string const& colname );
  double rms   ( sqlite3* db, std::string const& tname, std::string const& colname );

} //namespace sqlite
#endif /* art_Ntuple_sqlite_helpers_h */

// Local Variables:
// mode: c++
// End:
