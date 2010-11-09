//////////////////////////////////////////////////////////////////////
//
// Original Author: Luca Lista
// Current Author: Bill Tanenbaum
//
//////////////////////////////////////////////////////////////////////


#include "art/Framework/IO/Catalog/InputFileCatalog.h"

#include "art/Framework/IO/Catalog/SiteLocalConfig.h"
#include "art/Framework/Services/Registry/Service.h"
#include "cetlib/exception.h"

#include "boost/algorithm/string.hpp"
#include "fhiclcpp/ParameterSet.h"


namespace art {

  InputFileCatalog::InputFileCatalog(fhicl::ParameterSet const& pset,
                                     PoolCatalog & poolcat,
                                     std::string const& namesParameter,
                                     bool canBeEmpty, bool noThrow) :
    FileCatalog(poolcat),
    logicalFileNames_(canBeEmpty ?
        pset.get<std::vector<std::string> >(namesParameter, std::vector<std::string>()) :
        pset.get<std::vector<std::string> >(namesParameter)),
    fileNames_(logicalFileNames_),
    fileCatalogItems_() {

    if (logicalFileNames_.empty()) {
      if (canBeEmpty) return;
      throw art::Exception(art::errors::Configuration, "InputFileCatalog::InputFileCatalog()\n")
          << "Empty '" << namesParameter << "' parameter specified for input source.\n";
    }
    // Starting the catalog will write a catalog out if it does not exist.
    // So, do not start (or even read) the catalog unless it is needed.

    fileCatalogItems_.reserve(fileNames_.size());
    typedef std::vector<std::string>::iterator iter;
    for(iter it = fileNames_.begin(), lt = logicalFileNames_.begin(), itEnd = fileNames_.end();
        it != itEnd; ++it, ++lt) {
      boost::trim(*it);
      if (it->empty()) {
        throw art::Exception(art::errors::Configuration, "InputFileCatalog::InputFileCatalog()\n")
          << "An empty string specified in '" << namesParameter << "' parameter for input source.\n";
      }
      if (isPhysical(*it)) {
        lt->clear();
      }
      fileCatalogItems_.push_back(FileCatalogItem(*it, *lt));
    }
  }

  InputFileCatalog::~InputFileCatalog() {}

  void InputFileCatalog::findFile(std::string & pfn, std::string const& lfn, bool noThrow) {
    cet::exception("You cannot do a logical file lookup! (InputFileCatalog::findFile");
  }

}  // namespace art
