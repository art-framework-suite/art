#ifndef art_Framework_Core_ModuleType_h
#define art_Framework_Core_ModuleType_h
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"

#include <string>

namespace art {

class EDAnalyzer;
class EDFilter;
class OutputModule;
class EDProducer;

enum class ModuleType : int {
  NON_ART,
  PRODUCER,
  FILTER,
  ANALYZER,
  OUTPUT
};

enum class ModuleThreadingType : int {
    ILLEGAL // 0
  , LEGACY // 1
  , ONE // 2
  , STREAM // 3
  , GLOBAL // 4
};

inline
bool
is_modifier(ModuleType const& mt)
{
  return (mt == ModuleType::PRODUCER) || (mt == ModuleType::FILTER);
}

inline
bool
is_observer(ModuleType const& mt)
{
  return (mt == ModuleType::ANALYZER) || (mt == ModuleType::OUTPUT);
}

inline
std::string
ModuleType_to_string(ModuleType mt)
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
      return "output";
    default:
      throw Exception(errors::LogicError)
          << "Unable to find string for unrecognized ModuleType value "
          << static_cast<int>(mt)
          << ".\n";
  }
}

//inline
//std::string
//to_string(ModuleThreadingType mt)
//{
//  switch (mt) {
//    case ModuleThreadingType::ILLEGAL:
//      return "illegal";
//    case ModuleThreadingType::LEGACY:
//      return "legacy";
//    case ModuleThreadingType::ONE:
//      return "one";
//    case ModuleThreadingType::STREAM:
//      return "stream";
//    case ModuleThreadingType::GLOBAL:
//      return "global";
//    default:
//      throw Exception(errors::LogicError)
//          << "Unable to find string for unrecognized ModuleThreadingType value "
//          << static_cast<int>(mt)
//          << ".\n";
//  }
//}

} // namespace art

#endif /* art_Framework_Core_ModuleType_h */

// Local Variables:
// mode: c++
// End:
