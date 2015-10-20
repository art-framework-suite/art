#ifndef art_Framework_Services_Optional_detail_SimpleMemoryCheckConfig_h
#define art_Framework_Services_Optional_detail_SimpleMemoryCheckConfig_h

// ======================================================================
//
// SimpleMemoryCheckConfig
//
// ======================================================================

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"

namespace art {
  namespace detail {
    struct SMCheckConfig {
      fhicl::Atom<unsigned> ignoreTotal { fhicl::Name("ignoreTotal"), 1 };
      fhicl::Atom<bool> truncateSummary { fhicl::Name("truncateSummary"), true };
      fhicl::Atom<bool> showMallocInfo  { fhicl::Name("showMallocInfo"), false };
      fhicl::Atom<bool> oncePerEventMode { fhicl::Name("oncePerEventMode"), false };
      fhicl::Atom<bool> moduleMemorySummary { fhicl::Name("moduleMemorySummary"), false };
    };
  }
}

#endif /* art_Framework_Services_Optional_detail_SimpleMemoryCheckConfig_h */

// Local variables:
// mode: c++
// End:
