#include "art/Framework/Core/detail/ModuleKeyAndType.h"

namespace art::detail {
  ModuleType
  module_type(std::string const& full_key)
  {
    if (full_key.find("physics.producers") == 0) {
      return ModuleType::producer;
    } else if (full_key.find("physics.filters") == 0) {
      return ModuleType::filter;
    } else if (full_key.find("physics.analyzers") == 0) {
      return ModuleType::analyzer;
    } else if (full_key.find("outputs") == 0) {
      return ModuleType::output_module;
    }
    return ModuleType::non_art;
  }
}
