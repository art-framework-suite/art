#ifndef art_Persistency_Provenance_ModuleType_h
#define art_Persistency_Provenance_ModuleType_h
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"

#include <string>

namespace art {

  enum class ModuleType { non_art, producer, filter, analyzer, output_module };
  enum class ModuleThreadingType { illegal, legacy, shared, replicated };

  inline bool
  is_modifier(ModuleType const mt)
  {
    return (mt == ModuleType::producer) || (mt == ModuleType::filter);
  }

  inline bool
  is_observer(ModuleType const mt)
  {
    return (mt == ModuleType::analyzer) || (mt == ModuleType::output_module);
  }

  inline std::string
  to_string(ModuleType const mt)
  {
    switch (mt) {
      case ModuleType::non_art:
        return "non-art";
      case ModuleType::producer:
        return "producer";
      case ModuleType::filter:
        return "filter";
      case ModuleType::analyzer:
        return "analyzer";
      case ModuleType::output_module:
        return "output module";
      default:
        throw Exception(errors::LogicError)
          << "Unable to find string for unrecognized ModuleType value "
          << static_cast<int>(mt) << ".\n";
    }
  }

} // namespace art

#endif /* art_Persistency_Provenance_ModuleType_h */

// Local Variables:
// mode: c++
// End:
