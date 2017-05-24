#ifndef art_Framework_Art_BasicOutputOptionsHandler_h
#define art_Framework_Art_BasicOutputOptionsHandler_h

#include "art/Framework/Art/OptionsHandler.h"

// Handle the file input options: source, source-list, etc.

namespace art {
  class BasicOutputOptionsHandler;
}

class art::BasicOutputOptionsHandler : public art::OptionsHandler {
public:
  explicit BasicOutputOptionsHandler(bpo::options_description& desc);
private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const& vm) override;
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const& vm,
                       fhicl::intermediate_table& raw_config) override;

  std::string tmpDir_{};
};
#endif /* art_Framework_Art_BasicOutputOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
