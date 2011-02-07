#ifndef art_Framework_IO_Sources_EDInputSource_h
#define art_Framework_IO_Sources_EDInputSource_h

// ======================================================================
//
// EDInputSource
//
// ======================================================================

#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  class InputSourceDescription;

  class EDInputSource : public InputSource {
  public:
    explicit EDInputSource(fhicl::ParameterSet const& pset,
                           InputSourceDescription const& desc);
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
    virtual void setSubRun(SubRunNumber_t sr);


    InputFileCatalog catalog_;
    InputFileCatalog secondaryCatalog_;
  };

}  // art

// ======================================================================

#endif /* art_Framework_IO_Sources_EDInputSource_h */

// Local Variables:
// mode: c++
// End:
