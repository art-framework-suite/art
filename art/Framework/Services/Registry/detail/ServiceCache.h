#ifndef art_Framework_Services_Registry_detail_ServiceCache_h
#define art_Framework_Services_Registry_detail_ServiceCache_h

#include "art/Utilities/TypeID.h"

#include <map>

namespace art {
  namespace detail {
    class ServiceCacheEntry;

    typedef  std::map< TypeID, detail::ServiceCacheEntry >  ServiceCache;

  }

}
#endif /* art_Framework_Services_Registry_detail_ServiceCache_h */

// Local Variables:
// mode: c++
// End:
