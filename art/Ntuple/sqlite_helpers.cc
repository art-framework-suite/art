// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <assert.h>
#include <cmath>

#include "art/Ntuple/sqlite_helpers.h"

namespace sqlite
{

  namespace detail
  {

    //=======================================================================
    int getResult(void* data, int ncols [[gnu::unused]], char** results, char** /*cnames*/)
    {
      assert(ncols == 1);
      query_result* j = static_cast<query_result*>(data);
      j->count += 1;
      j->ddl.push_back( results[0] );
      return 0;
    }

    //=======================================================================
    void exec(sqlite3* db,std::string const& ddl)
    {
      char* errmsg = nullptr;
      if ( sqlite3_exec(db, ddl.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK )
        {
          std::string msg(errmsg);
          sqlite3_free(errmsg);
          throw std::runtime_error(msg);
        }
    }

    //=======================================================================
    query_result query(sqlite3* db,std::string const& ddl)
    {
      query_result res;
      char* errmsg = nullptr;
      if ( sqlite3_exec(db, ddl.c_str(), detail::getResult, &res, &errmsg) != SQLITE_OK )
        {
          std::string msg(errmsg);
          sqlite3_free(errmsg);
          throw std::runtime_error(msg);
        }
      return res;
    }

    //=========================================================================
    /// hasTable(db, name, cnames) returns true if the db has a
    /// table named 'name', with colums named 'cns' suitable for carrying a
    /// tuple<ARGS...>. It returns false if there is no table of that name, and
    /// throws an exception if there is a table of the given name but it
    /// does not match both the given column names and column types.
    bool hasTable(sqlite3* db, std::string const& name, std::string const& sqlddl)
    {
      std::string cmd("select sql from sqlite_master where type=\"table\" and name=\"");
      cmd += name;
      cmd += '"';

      detail::query_result const res = query(db, cmd);

      if (res.count == 0) { return false; }
      if (res.count == 1 && res.ddl[0] == sqlddl) { return true; }
      throw std::runtime_error("Existing database table name does not match description");
    }

  } // namespace detail

  //=======================================================================
  sqlite3* openDatabaseFile(std::string const& filename)
  {
    sqlite3* db = nullptr;
    int const rc = sqlite3_open_v2(filename.c_str(),
                                   &db,
                                   SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,
                                   nullptr);
    if (rc != SQLITE_OK) throw std::runtime_error("Failed to open SQLite database");

    return db;
  }

  //=======================================================================
  void deleteTable( sqlite3* db, std::string const& tname )
  {
    detail::exec( db, "delete from "s + tname );
  }

  void dropTable( sqlite3* db, std::string const& tname )
  {
    detail::exec( db, "drop table "s+ tname );
  }

  //=======================================================================
  // Statistics helpers

  double mean( sqlite3* db, const std::string& tname, const std::string& colname ){
    return query_db<double>( db, "select avg("s+colname+") from "s + tname );
  }

  double median( sqlite3* db, const std::string& tname, const std::string& colname ){
    return query_db<double>( db,
                             "select avg("s+colname+")"s+
                             " from (select "s+colname+
                             " from "s+tname+
                             " order by "s+colname+
                             " limit 2 - (select count(*) from "s + tname+") % 2"s+
                             " offset (select (count(*) - 1) / 2"s+
                             " from "s + tname+"))"s );
  }

  double rms( sqlite3* db, const std::string& tname, const std::string& colname ){
    double const ms = query_db<double>( db,
                                        "select sum("s+
                                        "("s + colname + "-(select avg("s + colname + ") from "s + tname +"))"s +
                                        "*"s +
                                        "("s + colname + "-(select avg("s + colname + ") from "s + tname +"))"s +
                                        " ) /"s +
                                        "(count("s + colname +")) from "s + tname );
    return std::sqrt( ms );
  }

} // namespace sqlite

