#ifndef art_Framework_Services_Registry_detail_ServiceCache_h
#define art_Framework_Services_Registry_detail_ServiceCache_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "canvas/Utilities/TypeID.h"

#include <map>

namespace art {
  namespace detail {
    using ServiceCache = std::map<TypeID, detail::ServiceCacheEntry>;
  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceCache_h */

// Local Variables:
// mode: c++
// End:
