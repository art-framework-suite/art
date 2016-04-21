#ifndef art_Framework_IO_Root_detail_getFileContributors_h
#define art_Framework_IO_Root_detail_getFileContributors_h

#include "TFile.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <string>
#include <vector>

class sqlite3;

namespace art {
  namespace detail {

    RangeSet getContributors(sqlite3*,
                             std::string const& filename,
                             BranchType,
                             unsigned rangeSetID);

  }
}

#endif

// Local variables:
// mode: c++
// End:
