#ifndef asldkfjalsdkjfalskjdfasldkfj
#define asldkfjalsdkjfalskjdfasldkfj

#include "art/Framework/Core/detail/ModuleKeyAndType.h"

#include <map>
#include <string>

namespace art::detail {
  class EnabledModules {
  public:
    static EnabledModules none();
    explicit EnabledModules(keytype_for_name_t&& enabled_modules,
                            module_entries_for_ordered_path_t&& trigger_paths,
                            module_entries_for_ordered_path_t&& end_paths);

    keytype_for_name_t
    modules() const noexcept
    {
      return enabledModules_;
    }
    module_entries_for_ordered_path_t
    trigger_paths() const noexcept
    {
      return triggerPaths_;
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
  };
}

#endif
