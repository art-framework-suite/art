#ifndef art_Framework_Art_detail_md_collector_describe_h
#define art_Framework_Art_detail_md_collector_describe_h

#include "fhiclcpp/types/ConfigurationTable.h"

#include <string>

namespace art::detail {
  std::string describe(cet::exempt_ptr<fhicl::ConfigurationTable const> pb,
                       std::string const& prefix);
}

// Local variables:
// mode: c++
// End:
#endif /* art_Framework_Art_detail_md_collector_describe_h */
