#ifndef art_Ntuple_sqlite_helpers_h
#define art_Ntuple_sqlite_helpers_h

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include "sqlite3.h"

namespace sqlite
{

  template <size_t N> using name_array = std::array<std::string, N>;

  namespace detail
  {
    /// coltype<T> returns the string used to name the column type used
    /// to store an object of type T. There is no implementation of the
    /// general case; the template must be specialized for each
    /// supported type.
    template <class T> std::string coltype();

    /// createTableIfNeeded will create a table named 'tname' in the
    /// given database, with column types determined by the types in
    /// ARGS, and with column names specified by beginCol...endCol.
    /// It will throw an exception if a table of the given name, but
    /// with a different description, already exists in the database.
    template <class ...ARGS, class IT>
    void createTableIfNeeded(sqlite3* db,
                             std::string const& tname,
                             IT beginCol,
                             IT endCol);

    /// function template sql_ddl returns the SQL DDL 'CREATE TABLE'
    /// statement to declare a table named 'tname', suitable for storing
    /// tuples of type TUP, and with column names determined by the
    /// range [beginCol, endCol).
    template <class TUP, class IT>
    std::string sql_ddl(std::string const& tname, IT beginCol, IT endCol);


    /// struct BuildSQL a helper used in sql_ddl.
    template <class TUP, std::size_t I> struct BuildSQL;

    // Implementation details below.

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

    // BuildSQL is a helper struct, with function add_more. A struct is
    // needed because partial specialization of function templates is
    // not supported in C++11.
    template <class TUP, std::size_t I>
    struct BuildSQL
    {
      static constexpr std::size_t SIZE = std::tuple_size<TUP>::value;
      using result_t = typename std::tuple_element<I, TUP>::type;

      template <class IT>
      static void add_more(std::string& cmd,
                           IT beginCol, IT endCol)
      {
        BuildSQL<TUP,I-1>::add_more(cmd, beginCol, endCol);
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
      static void add_more(std::string& cmd,
                           IT beginCol, IT /* unused */)
      {
        cmd += *beginCol;
        cmd += " ";
        cmd += coltype<result_t>();
      }
    };

    template <class TUP, class IT>
    std::string sql_ddl(std::string const& tname,
                        IT beginCol,
                        IT endCol)
    {
      std::string cmd("CREATE TABLE ");
      cmd += tname;
      cmd += " ( ";
      BuildSQL<TUP, std::tuple_size<TUP>::value-1>::add_more(cmd, beginCol, endCol);
      cmd += " )";
      return cmd;
    }

    struct query_result
    {
      std::string ddl;
      int count = 0;
    };

    int get_name(void* data, int ncols [[gnu::unused]], char** results, char** /*cnames*/)
    {
      assert(ncols == 1);
      query_result* j = static_cast<query_result*>(data);
      j->count += 1;
      j->ddl = results[0];
      return 0;
    }

    /// hasTable<ARGS...>(db, name, cnames) returns true if the db has a
    /// table named 'name', with colums named 'cns' suitable for carrying a
    /// tuple<ARGS...>. It returns false if there is no table of that name, and
    /// throws an exception if there is a table of the given name but it
    /// does not match both the given column names and column types.
    template <class ... ARGS>
    bool hasTable(sqlite3* db,
                  std::string const& name,
                  std::string const& sqlddl)
    {
      std::string cmd("select sql from sqlite_master where type=\"table\" and name=\"");
      cmd += name;
      cmd += '"';
      char* errmsg = nullptr;
      detail::query_result res;
      int status = sqlite3_exec(db, cmd.c_str(), detail::get_name, &res, &errmsg);
      if (status != SQLITE_OK)
        {
          std::string msg(errmsg);
          sqlite3_free(errmsg);
          throw std::runtime_error(msg);
        }
      if (res.count == 0) { return false; }
      if (res.count == 1 && res.ddl == sqlddl) { return true; }
      throw std::runtime_error("Existing database table name does not match description");
    }

    /// createTable table will create the table. Make sure it doesn't
    /// already exist before calling this!
    template <class ... ARGS>
    void createTable(sqlite3* db,
                     std::string const& ddl)
    {
      char* errmsg = nullptr;
      int status = sqlite3_exec(db, ddl.c_str(), nullptr, nullptr, &errmsg);
      if (status != SQLITE_OK)
        {
          std::string msg(errmsg);
          sqlite3_free(errmsg);
          throw std::runtime_error(msg);
        }
    }
  } // namespace detail

  sqlite3*
  openDatabaseFile(std::string const& filename)
  {
    sqlite3* db = nullptr;
    int rc = sqlite3_open_v2(filename.c_str(),
                             &db,
                             SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,
                             nullptr);
    if (rc != SQLITE_OK) throw std::runtime_error("Failed to open SQLite database");
    return db;
  }

  template <class ...ARGS, class IT>
  void createTableIfNeeded(sqlite3* db,
                           sqlite3_int64& rowid,
                           std::string const& tname,
                           IT beginCol,
                           IT endCol)
  {
    std::string sqlddl = detail::sql_ddl<std::tuple<ARGS...>>(tname, beginCol, endCol);
    if (!detail::hasTable<ARGS...>(db, tname, sqlddl)) {
      detail::createTable<ARGS...>(db, sqlddl);
    }
    else {
      //  Get last rowid of table
      std::string cmd("select count(*) from ");
      cmd += tname;
      char* errmsg = nullptr;
      sqlite::detail::query_result res;
      int status = sqlite3_exec(db,cmd.c_str(),detail::get_name, &res, &errmsg );
      if (status != SQLITE_OK)
        {
          std::string msg(errmsg);
          sqlite3_free(errmsg);
          throw std::runtime_error(msg);
        }
      rowid = std::stoul( res.ddl );
    }
  }

}
#endif /* art_Ntuple_sqlite_helpers_h */

// Local Variables:
// mode: c++
// End:
