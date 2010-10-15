//////////////////////////////////////////////////////////////////////
//
//
//
// Original Author: Luca Lista
// Current Author: Bill Tanenbaum
//
//////////////////////////////////////////////////////////////////////

#include "art/Framework/IO/Catalog/FileCatalog.h"

namespace art {

  FileCatalog::FileCatalog(PoolCatalog & poolcat) :
      catalog_(poolcat.catalog_),
      url_(),
      active_(false) {
  }

  FileCatalog::~FileCatalog() {
    if (active_) catalog_.commit();
    catalog_.disconnect();
  }

  void FileCatalog::commitCatalog() {
    catalog_.commit();
    catalog_.start();
  }

}
