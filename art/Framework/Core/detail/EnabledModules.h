#ifndef art_Framework_Core_detail_EnabledModules_h
#define art_Framework_Core_detail_EnabledModules_h

#include "art/Framework/Core/detail/ModuleKeyAndType.h"

#include <string>
#include <vector>

namespace art::detail {
  class EnabledModules {
  public:
    static EnabledModules none();
    explicit EnabledModules(keytype_for_name_t&& enabled_modules,
                            module_entries_for_ordered_path_t&& trigger_paths,
                            module_entries_for_ordered_path_t&& end_paths,
                            bool trigger_paths_override,
                            bool end_paths_override);

    keytype_for_name_t const&
    modules() const noexcept
    {
      return enabledModules_;
    }

    module_entries_for_ordered_path_t const&
    trigger_path_specs() const noexcept
    {
      return triggerPaths_;
    }

    bool
    trigger_paths_override() const noexcept
    {
      return triggerPathsOverride_;
    }
    bool
    end_paths_override() const noexcept
    {
      return endPathsOverride_;
    }

    std::vector<std::string> trigger_path_names() const;

    module_entries_for_ordered_path_t
    end_paths() const noexcept
    {
      return endPaths_;
    }

  private:
    EnabledModules() = default;
    keytype_for_name_t enabledModules_{};
    module_entries_for_ordered_path_t triggerPaths_{};
    module_entries_for_ordered_path_t endPaths_{};
    bool triggerPathsOverride_{false};
    bool endPathsOverride_{false};
  };
}

#endif /* art_Framework_Core_detail_EnabledModules_h */

// Local Variables:
// mode: c++
// End:
