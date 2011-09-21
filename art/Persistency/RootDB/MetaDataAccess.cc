#include "art/Persistency/RootDB/MetaDataAccess.h"

#include "art/Persistency/RootDB/trace_sqldb.h"
#include "art/Utilities/Exception.h"

art::MetaDataAccess &
art::MetaDataAccess::instance()
{
  static MetaDataAccess me;
  return me;
}

void
art::MetaDataAccess::setTracing(bool onOff)
{
  if (onOff && !tracing_) {
    trace_sqldb(dbHandle_, onOff, ":memory:");
  }
  tracing_ = onOff;
}

art::MetaDataAccess::MetaDataAccess()
  :
  dbHandle_(":memory:", SQLITE_OPEN_CREATE),
  tracing_(false)
{
  char const * debug_sql = getenv("ART_DEBUG_SQL");
  if (debug_sql != nullptr) {
    setTracing(true);
  }
}

art::MetaDataAccess::~MetaDataAccess()
{
}
