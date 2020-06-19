#include "art/Framework/Core/detail/EnabledModules.h"

namespace art::detail {
  EnabledModules
  EnabledModules::none()
  {
    return EnabledModules{};
  }

  EnabledModules::EnabledModules(
    keytype_for_name_t&& enabled_modules,
    module_entries_for_ordered_path_t&& trigger_paths,
    module_entries_for_ordered_path_t&& end_paths)
    : enabledModules_{std::move(enabled_modules)}
    , triggerPaths_{std::move(trigger_paths)}
    , endPaths_{std::move(end_paths)}
  {}

  std::vector<std::string>
  EnabledModules::trigger_path_names() const
  {
    std::vector<std::string> result;
    for (auto const& pr : triggerPaths_) {
      result.push_back(pr.first);
    }
    return result;
  }
}
