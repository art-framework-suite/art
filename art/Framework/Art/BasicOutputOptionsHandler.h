#ifndef art_Framework_Art_BasicOutputOptionsHandler_h
#define art_Framework_Art_BasicOutputOptionsHandler_h

// Handle the file input options: source, source-list, etc.

namespace art {
  class BasicOutputOptionsHandler;
}

#include "art/Framework/Art/OptionsHandler.h"

class art::BasicOutputOptionsHandler : public art::OptionsHandler {
public:
  explicit BasicOutputOptionsHandler(bpo::options_description & desc);
private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const & vm);
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const & vm,
                       fhicl::intermediate_table & raw_config);
};
#endif /* art_Framework_Art_BasicOutputOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
