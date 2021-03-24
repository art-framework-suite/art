#ifndef art_Utilities_ensureTable_h
#define art_Utilities_ensureTable_h

#include "fhiclcpp/fwd.h"

#include <string>

namespace art {
  void ensureTable(fhicl::intermediate_table& table,
                   std::string const& fhicl_spec);
}

#endif /* art_Utilities_ensureTable_h */

// Local Variables:
// mode: c++
// End:
