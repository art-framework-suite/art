#ifndef art_Framework_Services_Registry_detail_ensure_only_one_thread_h
#define art_Framework_Services_Registry_detail_ensure_only_one_thread_h

#include "fhiclcpp/fwd.h"

namespace art::detail {
  void ensure_only_one_thread(fhicl::ParameterSet const& service_pset);
}

#endif /* art_Framework_Services_Registry_detail_ensure_only_one_thread_h */

// Local Variables:
// mode: c++
// End:
