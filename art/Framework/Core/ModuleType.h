#ifndef art_Framework_Core_ModuleType_h
#define art_Framework_Core_ModuleType_h

////////////////////////////////////////////////////////////////////////
// Enum and associated utilities to help with module classification and
// construction.
//
////////////////////////////////////////////////////////////////////////

#include "canvas/Utilities/Exception.h"

#include <string>

namespace art {
  // Forward declarations for specializations.
  class EDAnalyzer;
  class EDFilter;
  class OutputModule;
  class EDProducer;

  // ModuleType.
  enum class ModuleType { NON_ART, ANALYZER, FILTER, PRODUCER, OUTPUT };

  std::string to_string(ModuleType mt);

  // Useful functions.
  bool is_observer(ModuleType const& mt);
  bool is_modifier(ModuleType const& mt);
}

inline std::string
art::to_string(ModuleType mt)
{
  switch (mt) {
    case ModuleType::NON_ART:
      return "non-art";
    case ModuleType::ANALYZER:
      return "analyzer";
    case ModuleType::FILTER:
      return "filter";
    case ModuleType::PRODUCER:
      return "producer";
    case ModuleType::OUTPUT:
      return "output";
    default:
      throw Exception(errors::LogicError)
        << "Unable to find string for unrecognized ModuleType value "
        << static_cast<int>(mt) << ".\n";
  }
}

inline bool
art::is_observer(ModuleType const& mt)
{
  return mt == ModuleType::ANALYZER || mt == ModuleType::OUTPUT;
}

inline bool
art::is_modifier(ModuleType const& mt)
{
  return mt == ModuleType::FILTER || mt == ModuleType::PRODUCER;
}
#endif /* art_Framework_Core_ModuleType_h */

// Local Variables:
// mode: c++
// End:
