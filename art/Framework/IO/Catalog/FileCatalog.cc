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

  FileCatalog::FileCatalog() :
    url_(),
    active_(false) {
  }

  FileCatalog::~FileCatalog() {
  }

  void FileCatalog::commitCatalog() {
  }

}
