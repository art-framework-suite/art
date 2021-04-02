#ifndef art_Framework_Art_detail_prune_configuration_h
#define art_Framework_Art_detail_prune_configuration_h

#include "art/Framework/Core/detail/EnabledModules.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/fwd.h"

#include <vector>

namespace art::detail {
  EnabledModules prune_config_if_enabled(bool prune_config,
                                         bool report_enabled,
                                         fhicl::intermediate_table& config);
  std::vector<ModuleSpec> sequence_to_entries(
    fhicl::extended_value::sequence_t const& seq,
    bool allow_nil_entries = false);

  std::vector<PathSpec> path_specs(
    std::vector<ModuleSpec> const& selection_override_entries,
    std::string const& path_selection_override);
}

#endif /* art_Framework_Art_detail_prune_configuration_h */

// Local Variables:
// mode: c++
// End:
