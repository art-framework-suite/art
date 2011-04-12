#ifndef art_Framework_IO_Sources_EDInputSource_h
#define art_Framework_IO_Sources_EDInputSource_h

// ======================================================================
//
// EDInputSource
//
// ======================================================================

#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
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

  class EDInputSource : public DecrepitRelicInputSourceImplementation {
  public:
    explicit EDInputSource(fhicl::ParameterSet const& pset,
                           InputSourceDescription const& desc);

    virtual ~EDInputSource();

    std::vector<FileCatalogItem> const& fileCatalogItems() const
    {
      return catalog_.fileCatalogItems();
    }

    InputFileCatalog& catalog() {return catalog_;}

  private:
    InputFileCatalog catalog_;
  };

}  // art

// ======================================================================

#endif /* art_Framework_IO_Sources_EDInputSource_h */

// Local Variables:
// mode: c++
// End:
