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

    bool
    operator==(ModuleKeyAndType const& other) const
    {
      return key == other.key && type == other.type;
    }

    bool
    operator!=(ModuleKeyAndType const& other) const
    {
      return !operator==(other);
    }
  };

  ModuleType module_type(std::string const& full_key);
  using modules_per_path_t =
    std::map<std::string, std::vector<ModuleKeyAndType>>;
}

#endif
