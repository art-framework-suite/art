#ifndef art_Framework_Core_detail_ModuleKeyAndType_h
#define art_Framework_Core_detail_ModuleKeyAndType_h

#include "art/Persistency/Provenance/ModuleType.h"

#include <map>
#include <string>
#include <vector>

namespace art::detail {

  struct ModuleKeyAndType {
    std::string key;
    ModuleType type;
  };

  enum class FilterAction { Normal = 0, Ignore = 1, Veto = 2 };
  struct PathEntry {
    std::string name;
    FilterAction action;
  };

  inline bool
  operator==(ModuleKeyAndType const& a, ModuleKeyAndType const& b) noexcept
  {
    return a.key == b.key && a.type == b.type;
  }

  inline bool
  operator!=(ModuleKeyAndType const& a, ModuleKeyAndType const& b) noexcept
  {
    return not(a == b);
  }

  ModuleType module_type(std::string const& full_key);
  using keytype_for_name_t = std::map<std::string, ModuleKeyAndType>;
  using module_entries_for_ordered_path_t =
    std::vector<std::pair<std::string, std::vector<PathEntry>>>;
  using module_entries_for_path_t =
    std::map<std::string, std::vector<PathEntry>>;
  using modules_for_path_t =
    std::map<std::string, std::vector<ModuleKeyAndType>>;
}

#endif /* art_Framework_Core_detail_ModuleKeyAndType_h */

// Local Variables:
// mode: c++
// End:
