#include "art/Persistency/RootDB/SQLErrMsg.h"

#include "art/Utilities/Exception.h"

extern "C" {
#include "sqlite3.h"
}

void
art::SQLErrMsg::throwIfError() const
{
  if (errMsg_) {
    throw Exception(errors::SQLExecutionError,
                    "SQLite3Wrapper::exec")
        << "Error executing SQL: "
        << errMsg_
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
