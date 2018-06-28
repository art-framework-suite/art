#ifndef art_Framework_Core_detail_ModuleConfigInfo_h
#define art_Framework_Core_detail_ModuleConfigInfo_h
// vim: set sw=2 expandtab :

#include "art/Persistency/Provenance/ModuleType.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {
  namespace detail {
    struct ModuleConfigInfo {
      std::string moduleLabel;
      std::string configTableName;
      ModuleType moduleType;
      ModuleThreadingType moduleThreadingType;
      fhicl::ParameterSet modPS;
      std::string libSpec;
    };
  }
}

#endif /* art_Framework_Core_detail_ModuleConfigInfo_h */

// Local Variables:
// mode: c++
// End:
