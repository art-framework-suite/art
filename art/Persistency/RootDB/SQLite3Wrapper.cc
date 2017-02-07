#include "art/Persistency/RootDB/SQLite3Wrapper.h"

#include "art/Persistency/RootDB/tkeyvfs.h"
#include "art/Persistency/RootDB/SQLErrMsg.h"
#include "canvas/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <cstddef>

namespace {

  enum DBTYPE {
    SQLITE3_TKEYDB,
    SQLITE3_MEMDB,
    SQLITE3_FILEDB,
    SQLITE3_TMPDB
  };

  std::string
  dbTypeAsString(DBTYPE type)
  {
    std::string result;
    switch (type) {
      case SQLITE3_TKEYDB:
        result = "tkeyvfs";
        break;
      case SQLITE3_MEMDB:
        result = "in-memory";
        break;
      case SQLITE3_FILEDB:
        result = "file";
        break;
      case SQLITE3_TMPDB:
        result = "tmp";
        break;
      default:
        result = "UNKNOWN";
        break;
    }
    return result;
  }

  void traceit(void * string_ptr, const char * zSQL)
  {
    assert(string_ptr);
    mf::LogAbsolute("SQLTrace")
        << "+ SQLTrace ("
        << *reinterpret_cast<std::string*>(string_ptr)
        << "): "
        << zSQL;
  }
}

art::SQLite3Wrapper::
SQLite3Wrapper()
  :
  db_(0),
  key_()
{
}

art::SQLite3Wrapper::
SQLite3Wrapper(std::string const & key,
               int flags)
  :
  db_(0),
  key_(key)
{
  initDB(flags);
}

art::SQLite3Wrapper::
SQLite3Wrapper(TFile * tfile,
               std::string const & key,
               int flags)
  :
  db_(0),
  key_(key)
{
  initDB(flags, tfile);
}

bool
art::SQLite3Wrapper::wantTracing()
{
  static char const * s_debug = getenv("ART_DEBUG_SQL");
  return s_debug;
}

void
art::SQLite3Wrapper::
reset()
{
  SQLite3Wrapper tmp;
  swap(tmp);
}

void
art::SQLite3Wrapper::
reset(std::string const & key,
      int flags)
{
  SQLite3Wrapper tmp{key, flags};
  swap(tmp);
}

void
art::SQLite3Wrapper::
reset(TFile * tfile,
      std::string const & key,
      int flags)
{
  SQLite3Wrapper tmp{tfile, key, flags};
  swap(tmp);
}

art::SQLite3Wrapper::
~SQLite3Wrapper()
{
  if (db_) { sqlite3_close(db_); }
}

void
art::SQLite3Wrapper::
swap(SQLite3Wrapper & other)
{
  using std::swap;
  swap(db_, other.db_);
  swap(key_, other.key_);
}

void
art::SQLite3Wrapper::
initDB(int flags, TFile * tfile)
{
  int err = 0;
  DBTYPE type =
    tfile ? SQLITE3_TKEYDB :
    key_.size() ?
    (key_ == ":memory:" ? SQLITE3_MEMDB : SQLITE3_FILEDB) :
      SQLITE3_TMPDB;
  switch (type) {
    case SQLITE3_TKEYDB:
      if (!key_.size()) {
        throw Exception{errors::FileOpenError}
            << "Failed to open TKEYVFS DB due to empty key spec.\n";
      }
      err = tkeyvfs_open_v2(key_.c_str(), &db_, flags, tfile);
      break;
    case SQLITE3_MEMDB:
      err = sqlite3_open(key_.c_str(), &db_);
      break;
    case SQLITE3_FILEDB:
      err = sqlite3_open_v2(key_.c_str(), &db_, flags, nullptr);
      break;
    case SQLITE3_TMPDB:
      err = sqlite3_open(key_.c_str(), &db_);
      break;
  }
  if (err) {
    throw Exception{errors::FileOpenError}
        << "Failed to open requested DB, \""
        << key_
        << "\" of type, \""
        << dbTypeAsString(type)
        << "\" -- "
        << sqlite3_errmsg(db_)
        << "\n";
  }
  maybeTrace();
}

void
art::SQLite3Wrapper::maybeTrace() const
{
  if (wantTracing()) {
    // Cast away constness for C interface.
    sqlite3_trace(db_, traceit, const_cast<std::string *>(&key_));
  }
}
