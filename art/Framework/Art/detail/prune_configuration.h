#ifndef art_Framework_Art_detail_prune_configuration_h
#define art_Framework_Art_detail_prune_configuration_h

#include "art/Framework/Core/detail/EnabledModules.h"
#include "fhiclcpp/fwd.h"

namespace art::detail {
  EnabledModules prune_config_if_enabled(bool prune_config,
                                         bool report_enabled,
                                         fhicl::intermediate_table& config);
}

#endif /* art_Framework_Art_detail_prune_configuration_h */

// Local Variables:
// mode: c++
// End:
