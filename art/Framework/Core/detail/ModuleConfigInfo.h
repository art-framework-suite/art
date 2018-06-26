#ifndef art_Framework_Core_detail_ModuleConfigInfo_h
#define art_Framework_Core_detail_ModuleConfigInfo_h
// vim: set sw=2 expandtab :

#include "art/Persistency/Provenance/ModuleType.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {
  namespace detail {
    struct ModuleConfigInfo {
      std::string configTableName_;
      ModuleType moduleType_;
      ModuleThreadingType moduleThreadingType_;
      fhicl::ParameterSet modPS_;
      std::string libSpec_;
    };
  }
}

#endif /* art_Framework_Core_detail_ModuleConfigInfo_h */

// Local Variables:
// mode: c++
// End:
