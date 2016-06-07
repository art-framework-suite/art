#ifndef art_Framework_Art_EventProcessingOptionsHandler_h
#define art_Framework_Art_EventProcessingOptionsHandler_h

// Handle the file input options: source, source-list, etc.

#include "art/Framework/Art/OptionsHandler.h"

namespace art {

  class EventProcessingOptionsHandler : public OptionsHandler {
  public:
    explicit EventProcessingOptionsHandler(bpo::options_description& desc,
                                           bool rethrowDefault = false);
  private:
    // Check selected options for consistency.
    int doCheckOptions(bpo::variables_map const & vm) override;
    // Act on selected options.
    int doProcessOptions(bpo::variables_map const & vm,
                         fhicl::intermediate_table & raw_config) override;
    bool rethrowDefault_;
  };

}
#endif /* art_Framework_Art_DebugOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
