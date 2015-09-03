#ifndef art_Utilities_detail_serviceConfigLocation_h
#define art_Utilities_detail_serviceConfigLocation_h
#include <string>

namespace fhicl {
  class intermediate_table;
}

namespace art {
  namespace detail {
    std::string serviceConfigLocation(fhicl::intermediate_table & raw_config,
                                      std::string const & service);
  }
}
#endif /* art_Utilities_detail_serviceConfigLocation_h */

// Local Variables:
// mode: c++
// End:
