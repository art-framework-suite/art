#ifndef art_Framework_Art_ProcessingOptionsHandler_h
#define art_Framework_Art_ProcessingOptionsHandler_h

// Handle the file input options: source, source-list, etc.

#include "art/Framework/Art/OptionsHandler.h"

namespace art {

  class ProcessingOptionsHandler : public OptionsHandler {
  public:
    explicit ProcessingOptionsHandler(bpo::options_description& desc,
                                      bool rethrowDefault);

  private:
    // Check selected options for consistency.
    int doCheckOptions(bpo::variables_map const& vm) override;
    // Act on selected options.
    int doProcessOptions(bpo::variables_map const& vm,
                         fhicl::intermediate_table& raw_config) override;
    bool rethrowDefault_;
  };
}
#endif /* art_Framework_Art_ProcessingOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
