#include "art/Persistency/RootDB/SQLErrMsg.h"

#include "canvas/Utilities/Exception.h"

extern "C" {
#include "sqlite3.h"
}

void
art::SQLErrMsg::throwIfError()
{
  if (errMsg_) {
    std::string msg(errMsg_);
    reset();
    throw Exception(errors::SQLExecutionError,
                    "SQLite3Wrapper::exec")
        << "Error executing SQL: "
        << msg
        << "\n";
  }
}

void
art::SQLErrMsg::reset()
{
  sqlite3_free(errMsg_);
  errMsg_ = 0;
}

art::SQLErrMsg::~SQLErrMsg()
{
  reset();
}
