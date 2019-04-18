#ifndef art_Framework_Art_detail_prune_configuration_h
#define art_Framework_Art_detail_prune_configuration_h

#include "art/Framework/Core/detail/ModuleKeyAndType.h"

#include <map>
#include <string>
#include <vector>

namespace fhicl {
  class intermediate_table;
}

namespace art::detail {
  std::map<std::string, ModuleKeyAndType> prune_config_if_enabled(
    bool prune_config,
    fhicl::intermediate_table& config);
}

#endif /* art_Framework_Art_detail_prune_configuration_h */

// Local Variables:
// mode: c++
// End:
