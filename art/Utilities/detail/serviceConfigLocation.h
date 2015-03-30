#ifndef JJSHDJFHF
#define JJSHDJFHF
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
#endif

// Local Variables:
// mode: c++
// End:
