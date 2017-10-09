#ifndef art_Framework_Art_DebugOptionsHandler_h
#define art_Framework_Art_DebugOptionsHandler_h

#include "art/Framework/Art/OptionsHandler.h"
#include "art/Framework/Art/detail/DebugOutput.h"

// Handle the debugging options

namespace art {
  class DebugOptionsHandler;
}

class art::DebugOptionsHandler : public art::OptionsHandler {
public:
  explicit DebugOptionsHandler(bpo::options_description& desc,
                               std::string const& basename,
                               detail::DebugOutput& dbg);

private:
  // Check selected options for consistency.
  int doCheckOptions(bpo::variables_map const& vm) override;
  // Act on selected options.
  int doProcessOptions(bpo::variables_map const& vm,
                       fhicl::intermediate_table& raw_config) override;

  detail::DebugOutput& dbg_;
};
#endif /* art_Framework_Art_DebugOptionsHandler_h */

// Local Variables:
// mode: c++
// End:
