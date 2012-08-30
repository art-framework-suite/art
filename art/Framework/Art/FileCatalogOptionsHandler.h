#ifndef art_Framework_Art_FileCatalogOptionsHandler_h
#define art_Framework_Art_FileCatalogOptionsHandler_h

// Handle the file input options: source, source-list, etc.

namespace art {
  class FileCatalogOptionsHandler;
}

#include "art/Framework/Art/OptionsHandler.h"

#include <string>

class art::FileCatalogOptionsHandler : public art::OptionsHandler {
public:
  explicit FileCatalogOptionsHandler(bpo::options_description & desc);
private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const & vm);
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const & vm,
                       fhicl::intermediate_table & raw_config);

  // Data.
  bpo::options_description const & desc_;
  bool requireMetadata_;
  bool wantSAMweb_;
};
#endif /* art_Framework_Art_FileCatalogOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
