#ifndef art_Framework_Core_ModuleType_h
#define art_Framework_Core_ModuleType_h
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"

#include <string>

namespace art {

  enum class ModuleType : int { NON_ART, PRODUCER, FILTER, ANALYZER, OUTPUT };

  enum class ModuleThreadingType : int { ILLEGAL, LEGACY, SHARED, REPLICATED };

  inline bool
  is_modifier(ModuleType const mt)
  {
    return (mt == ModuleType::PRODUCER) || (mt == ModuleType::FILTER);
  }

  inline bool
  is_observer(ModuleType const mt)
  {
    return (mt == ModuleType::ANALYZER) || (mt == ModuleType::OUTPUT);
  }

  inline std::string
  ModuleType_to_string(ModuleType const mt)
  {
    switch (mt) {
      case ModuleType::NON_ART:
        return "non-art";
      case ModuleType::PRODUCER:
        return "producer";
      case ModuleType::FILTER:
        return "filter";
      case ModuleType::ANALYZER:
        return "analyzer";
      case ModuleType::OUTPUT:
        return "output module";
      default:
        throw Exception(errors::LogicError)
          << "Unable to find string for unrecognized ModuleType value "
          << static_cast<int>(mt) << ".\n";
    }
  }

} // namespace art

#endif /* art_Framework_Core_ModuleType_h */

// Local Variables:
// mode: c++
// End:
