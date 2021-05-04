#ifndef art_Framework_Core_detail_ModuleConfigInfo_h
#define art_Framework_Core_detail_ModuleConfigInfo_h
// vim: set sw=2 expandtab :

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art::detail {
  struct ModuleConfigInfo {
    ModuleDescription modDescription;
    fhicl::ParameterSet modPS;
    ModuleType moduleType;
  };
}

#endif /* art_Framework_Core_detail_ModuleConfigInfo_h */

// Local Variables:
// mode: c++
// End:
