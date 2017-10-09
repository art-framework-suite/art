#ifndef art_Framework_Services_Registry_detail_ServiceStack_h
#define art_Framework_Services_Registry_detail_ServiceStack_h

#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"

#include <stack>

namespace art {
  namespace detail {
    typedef std::stack<WrapperBase_ptr> ServiceStack;
  }
}
#endif /* art_Framework_Services_Registry_detail_ServiceStack_h */

// Local Variables:
// mode: c++
// End:
