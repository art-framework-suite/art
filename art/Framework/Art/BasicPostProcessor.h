#ifndef art_Framework_Art_BasicPostProcessor_h
#define art_Framework_Art_BasicPostProcessor_h

// Handle the basic options, like config, help, process-name.

namespace art {
  class BasicPostProcessor;
}

#include "art/Framework/Art/OptionsHandler.h"
#include "cetlib/filepath_maker.h"

#include <string>

class art::BasicPostProcessor : public art::OptionsHandler {
private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const & vm);
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const & vm,
                       fhicl::intermediate_table & raw_config);
};
#endif /* art_Framework_Art_BasicPostProcessor_h */

// Local Variables:
// mode: c++
// End:
