#ifndef art_Framework_Art_detail_describe_h
#define art_Framework_Art_detail_describe_h

#include "fhiclcpp/types/ConfigurationTable.h"

#include <string>

namespace art {
  namespace detail {
    std::string describe(cet::exempt_ptr<fhicl::ConfigurationTable const> pb,
                         std::string const& prefix);
  }
} // namespace art

// Local variables:
// mode: c++
// End:
#endif /* art_Framework_Art_detail_describe_h */
