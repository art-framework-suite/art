// =======================================================
//
// sqlite helpers
//
// =======================================================

#include <cassert>
#include <cmath>
#include <regex>

#include "art/Ntuple/sqlite_helpers.h"
#include "canvas/Utilities/Exception.h"

namespace {
  std::string normalize(std::string to_replace)
  {
    // Replace multiple spaces with 1 space.
    {
      std::regex const r {"\\s+"};
      to_replace = std::regex_replace(to_replace, r, " ");
    }
    // Ensure no spaces after commas
    {
      std::regex const r {", "};
      to_replace = std::regex_replace(to_replace, r, ",");
    }
    return to_replace;
  }
}

namespace sqlite {

  namespace detail {

    //=================================================================
    // hasTable(db, name, cnames) returns true if the db has a table
    // named 'name', with columns named 'cns' suitable for carrying a
    // tuple<ARGS...>. It returns false if there is no table of that
    // name, and throws an exception if there is a table of the given
    // name but it does not match both the given column names and
    // column types.
    bool hasTable(sqlite3* db, std::string const& name, std::string const& sqlddl)
    {
      std::string cmd {"select sql from sqlite_master where type=\"table\" and name=\""};
      cmd += name;
      cmd += '"';

      auto const res = query(db, cmd);

      if (res.empty())
        return false;

      if (res.data.size() != 1ull) {
        throw art::Exception(art::errors::SQLExecutionError)
          << "Problematic query: " << res.data.size() << " instead of 1.\n";
      }

      // This is a somewhat fragile way of validating schemas.  A
      // better way would be to rely on sqlite3's insertion facilities
      // to determine if an insert of in-memory data would be
      // compatible with the on-disk schema.  This would require
      // creating a temporary table (so as to avoid inserting then
      // deleting a dummy row into the desired table)according to the
      // on-disk schema, and inserting some default values according
      // to the requested schema.
      if (normalize(res.data[0][0]) == normalize(sqlddl))
        return true;

      throw art::Exception(art::errors::SQLExecutionError)
        << "Existing database table name does not match description:\n"
        << "   DDL on disk: " << res.data[0][0] << '\n'
        << "   Current DDL: " << sqlddl << '\n';
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
        throw art::Exception{art::errors::Configuration}
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
