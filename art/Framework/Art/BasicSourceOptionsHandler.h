#ifndef art_Framework_Art_BasicSourceOptionsHandler_h
#define art_Framework_Art_BasicSourceOptionsHandler_h

// Handle the file input options: source, source-list, etc.

namespace art {
  class BasicSourceOptionsHandler;
}

#include "art/Framework/Art/OptionsHandler.h"

#include <string>

class art::BasicSourceOptionsHandler : public art::OptionsHandler {
public:
  explicit BasicSourceOptionsHandler(bpo::options_description & desc);
private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const & vm);
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const & vm,
                       fhicl::intermediate_table & raw_config);

  // Private helper functions.
  bool processSourceListArg_(bpo::variables_map const & vm,
                             std::vector<std::string> & source_list);
};
#endif /* art_Framework_Art_BasicSourceOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
