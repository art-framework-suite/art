#ifndef art_Framework_Services_Registry_detail_ServiceCache_h
#define art_Framework_Services_Registry_detail_ServiceCache_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "canvas/Utilities/TypeID.h"

#include <map>

namespace art::detail {
  using ServiceCache = std::map<TypeID, ServiceCacheEntry>;
}

#endif /* art_Framework_Services_Registry_detail_ServiceCache_h */

// Local Variables:
// mode: c++
// End:
