// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <cassert>
#include <cmath>
#include <iostream>

#include "art/Ntuple/sqlite_helpers.h"
#include "canvas/Utilities/Exception.h"

namespace sqlite {

  namespace detail {

    //=================================================================
    // hasTable(db, name, cnames) returns true if the db has a table
    // named 'name', with colums named 'cns' suitable for carrying a
    // tuple<ARGS...>. It returns false if there is no table of that
    // name, and throws an exception if there is a table of the given
    // name but it does not match both the given column names and
    // column types.
    bool hasTable(sqlite3* db, std::string const& name, std::string const& sqlddl)
    {
      std::string cmd("select sql from sqlite_master where type=\"table\" and name=\"");
      cmd += name;
      cmd += '"';

      auto const res = query(db, cmd);

      if (res.empty()) { return false; }
      if (res.data.size() == 1 && res.data[0][0] == sqlddl) { return true; }
      throw art::Exception(art::errors::SQLExecutionError)
        << "Existing database table name does not match description";
    }

    //================================================================
    // The locking mechanisms for nfs systems are deficient and can
    // thus wreak havoc with sqlite, which depends upon them.  In
    // order to support an sqlite database on nfs, we use a URI,
    // explicitly including the query parameter: 'nolock=1'.  We will
    // have to revisit this choice once we consider multiple
    // processes/threads writing to the same database file.
    inline std::string assembleURI(std::string const& filename)
    {
      // Arbitrary decision: don't allow users to specify a URI since
      // they may (unintentionally) remove the 'nolock' parameter,
      // thus potentially causing issues with nfs.
      if (filename.substr(0,5) == "file:") {
        throw art::Exception(art::errors::Configuration)
          << "art does not allow an SQLite database filename that starts with 'file:'.\n"
          << "Please contact artists@fnal.gov if you believe this is an error.";
      }
      return "file:"+filename+"?nolock=1";
    }

  } // namespace detail

  sqlite3* openDatabaseFile(std::string const& filename)
  {
    sqlite3* db {nullptr};
    std::string const uri = detail::assembleURI(filename);
    int const rc = sqlite3_open_v2(uri.c_str(),
                                   &db,
                                   SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_URI,
                                   nullptr);
    if (rc != SQLITE_OK) {
      throw art::Exception{art::errors::SQLExecutionError}
        << "Failed to open SQLite database\n"
        << "Return code of: " << rc;
    }

    return db;
  }

  //=======================================================================
  void exec(sqlite3* db,std::string const& ddl)
  {
    char* errmsg {nullptr};
    if (sqlite3_exec(db, ddl.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
      std::string msg {errmsg};
      sqlite3_free(errmsg);
      throw art::Exception{art::errors::SQLExecutionError} << msg;
    }
  }

  //=======================================================================
  void deleteTable(sqlite3* db, std::string const& tname)
  {
    exec(db, "delete from "s + tname);
  }

  void dropTable(sqlite3* db, std::string const& tname)
  {
    exec(db, "drop table "s+ tname);
  }

  //=======================================================================
  // Statistics helpers

  double mean(sqlite3* db, std::string const& tname, std::string const& colname){
    double result {};
    auto r = query(db, "select avg("s+colname+") from " + tname);
    throw_if_empty(r) >> result;
    return result;
  }

  double median(sqlite3* db, std::string const& tname, std::string const& colname){
    double result {};
    auto r = query(db,
                      "select avg("s+colname+")"+
                      " from (select "+colname+
                      " from "+tname+
                      " order by "+colname+
                      " limit 2 - (select count(*) from " + tname+") % 2"+
                      " offset (select (count(*) - 1) / 2"+
                      " from " + tname+"))");
    throw_if_empty(r) >> result;
    return result;
  }

  double rms(sqlite3* db, std::string const& tname, std::string const& colname){
    double result {};
    auto r = query(db,
                      "select sum("s+
                      "(" + colname + "-(select avg(" + colname + ") from " + tname +"))" +
                      "*" +
                      "(" + colname + "-(select avg(" + colname + ") from " + tname +"))" +
                      " ) /" +
                      "(count(" + colname +")) from " + tname);
    throw_if_empty(r) >> result;
    return std::sqrt(result);
  }

  unsigned nrows(sqlite3* db, std::string const& tname)
  {
    unsigned result {};
    auto r = query(db,"select count(*) from "+tname+";");
    throw_if_empty(r) >> result;
    return result;
  }

} // namespace sqlite
