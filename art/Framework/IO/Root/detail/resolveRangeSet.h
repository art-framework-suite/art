#ifndef art_Framework_IO_Root_detail_resolveRangeSet_h
#define art_Framework_IO_Root_detail_resolveRangeSet_h

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

    // The RangeSetInfo struct is an intermediary structure between
    // querying the SQLite database and creating the RangeSet.  It
    // allows one to combine all the necessary information for a
    // RangeSet, without having to call 'merge' multiple times, which,
    // under the covers, calls collapse, which can be expensive.  By
    // collecting RangeSetInfo objects, and calling the 'update'
    // function, the appropriate RangeSet information can be collected
    // and then provided to the RangeSet c'tor, where a collapse is
    // done only once.

    struct RangeSetInfo {

      explicit RangeSetInfo(RunNumber_t const r, std::vector<EventRange>&& ers)
        : run{r}
        , ranges{std::move(ers)}
      {}

      RunNumber_t run {IDNumber<Level::Run>::invalid()};
      std::vector<EventRange> ranges {};

      bool is_invalid() const
      {
        return run == IDNumber<Level::Run>::invalid();
      }

      static RangeSetInfo invalid() { return RangeSetInfo{}; }

      void update(RangeSetInfo&& rsi)
      {
        if (run != rsi.run) {
          throw art::Exception(art::errors::LogicError)
            << "Cannot merge two ranges-of-validity with different run numbers: "
            << run << " vs. " << rsi.run << '\n'
            << "Please contact artists@fnal.gov.";
        }
        std::move(rsi.ranges.begin(),
                  rsi.ranges.end(),
                  std::back_inserter(ranges));
      }

    private:

      RangeSetInfo() = default;
    };

    RangeSetInfo resolveRangeSetInfo(sqlite3*,
                                     std::string const& filename,
                                     BranchType,
                                     unsigned RangeSetID);

    RangeSet resolveRangeSet(RangeSetInfo const& rs);

    RangeSet resolveRangeSet(sqlite3*,
                             std::string const& filename,
                             BranchType,
                             unsigned rangeSetID);

  }
}

#endif /* art_Framework_IO_Root_detail_resolveRangeSet_h */

// Local variables:
// mode: c++
// End:
