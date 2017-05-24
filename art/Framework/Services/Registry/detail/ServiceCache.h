#ifndef art_Framework_Services_Registry_detail_ServiceCache_h
#define art_Framework_Services_Registry_detail_ServiceCache_h

#include "canvas/Utilities/TypeID.h"
#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"

#include <map>

namespace art {
  namespace detail {
    using ServiceCache = std::map<TypeID, detail::ServiceCacheEntry>;
  }
}
#endif /* art_Framework_Services_Registry_detail_ServiceCache_h */

// Local Variables:
// mode: c++
// End:
