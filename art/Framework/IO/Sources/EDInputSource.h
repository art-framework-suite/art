#ifndef FWCore_Sources_EDInputSource_h
#define FWCore_Sources_EDInputSource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include <vector>
#include <string>

namespace edm {
  class InputSourceDescription;
  class ParameterSet;
  class EDInputSource : public InputSource {
  public:
    explicit EDInputSource(ParameterSet const& pset, InputSourceDescription const& desc);
    virtual ~EDInputSource();

    std::vector<std::string> const& logicalFileNames(int n = 0) const {
      return n ? secondaryCatalog_.logicalFileNames() : catalog_.logicalFileNames();
    }
    std::vector<std::string> const& fileNames(int n = 0) const {
      return n ? secondaryCatalog_.fileNames() : catalog_.fileNames();
    }
    std::vector<FileCatalogItem> const& fileCatalogItems(int n = 0) const {
      return n ? secondaryCatalog_.fileCatalogItems() : catalog_.fileCatalogItems();
    }
    InputFileCatalog& catalog(int n = 0) {return n ? secondaryCatalog_ : catalog_;}

  private:
    virtual void setRun(RunNumber_t);
    virtual void setSubRun(SubRunNumber_t lb);

    PoolCatalog poolCatalog_;
    InputFileCatalog catalog_;
    InputFileCatalog secondaryCatalog_;
  };
}
#endif
