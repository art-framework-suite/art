#ifndef art_Persistency_Provenance_orderedProcessNamesCollection_h
#define art_Persistency_Provenance_orderedProcessNamesCollection_h

#include "canvas/Persistency/Provenance/ProcessHistory.h"

#include <string>
#include <vector>

namespace art {
  namespace detail {

    std::vector<std::vector<std::string>> orderedProcessNamesCollection(ProcessHistoryMap const& pHistMap);

  }
}

#endif /* art_Persistency_Provenance_orderedProcessNamesCollection_h */

// Local variables:
// mode: c++
// End:
