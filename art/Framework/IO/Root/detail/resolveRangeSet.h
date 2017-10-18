#ifndef art_Framework_IO_Root_detail_resolveRangeSet_h
#define art_Framework_IO_Root_detail_resolveRangeSet_h

#include "art/Framework/IO/Root/detail/RangeSetInfo.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <string>

class sqlite3;

namespace art {
  namespace detail {

    RangeSetInfo resolveRangeSetInfo(sqlite3*,
                                     std::string const& filename,
                                     BranchType,
                                     unsigned RangeSetID,
                                     bool compact);

    RangeSet resolveRangeSet(RangeSetInfo const& rs);

    RangeSet resolveRangeSet(sqlite3*,
                             std::string const& filename,
                             BranchType,
                             unsigned rangeSetID,
                             bool compact);
  }
}

#endif /* art_Framework_IO_Root_detail_resolveRangeSet_h */

// Local variables:
// mode: c++
// End:
