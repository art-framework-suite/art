#ifndef art_Framework_IO_Root_detail_RangeSetInfo_h
#define art_Framework_IO_Root_detail_RangeSetInfo_h

#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/IDNumber.h"

#include <vector>

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

      explicit RangeSetInfo(RunNumber_t const r, std::vector<EventRange>&& ers);

      bool is_invalid() const;
      static RangeSetInfo
      invalid()
      {
        return RangeSetInfo{};
      }
      void update(RangeSetInfo&& rsi, bool compact);

      RunNumber_t run{IDNumber<Level::Run>::invalid()};
      std::vector<EventRange> ranges{};

    private:
      RangeSetInfo() = default;
    };
  }
}

#endif /* art_Framework_IO_Root_detail_resolveRangeSet_h */

// Local variables:
// mode: c++
// End:
