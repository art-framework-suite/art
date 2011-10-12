#ifndef art_Framework_Core_detail_wrapLibraryManagerException_h
#define art_Framework_Core_detail_wrapLibraryManagerException_h

#include "art/Utilities/Exception.h"
#include <string>

namespace art {
  namespace detail {
    void wrapLibraryManagerException(art::Exception const & e,
                                     std::string const & item_type,
                                     std::string const & libspec,
                                     std::string const & release);
  }
}

#endif /* art_Framework_Core_detail_wrapLibraryManagerException_h */

// Local Variables:
// mode: c++
// End:
