#include "art/Persistency/RootDB/MetaDataAccess.h"

#include "art/Utilities/Exception.h"

art::MetaDataAccess &
art::MetaDataAccess::instance()
{
  static MetaDataAccess me;
  return me;
}

art::MetaDataAccess::MetaDataAccess()
  :
  dbHandle_(":memory:", SQLITE_OPEN_CREATE)
{
}

art::MetaDataAccess::~MetaDataAccess()
{
}
