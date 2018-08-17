#ifndef art_Framework_Core_detail_verify_names_h
#define art_Framework_Core_detail_verify_names_h

#include <string>

namespace art::detail {
  void verifyInstanceName(std::string const& in);
  void verifyFriendlyClassName(std::string const& fcn);
}

#endif /* art_Framework_Core_detail_verify_names_h */

// Local variables:
// mode: c++
// End:
