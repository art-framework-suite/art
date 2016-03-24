#ifndef art_Framework_IO_Root_detail_getFileContributors_h
#define art_Framework_IO_Root_detail_getFileContributors_h

#include "TFile.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <string>
#include <vector>

class sqlite3;

namespace art {
  namespace detail {

    std::vector<EventRange> getFileContributors();
    std::vector<EventRange> getRunContributors(TFile&, art::RunNumber_t);
    std::vector<EventRange> getSubRunContributors(TFile&, art::RunNumber_t, art::SubRunNumber_t);
    std::vector<EventRange> getSubRunContributors(sqlite3*, std::string const&,
                                                  art::RunNumber_t, art::SubRunNumber_t);
    RangeSet getSubRunContributors(sqlite3*,
                                   std::string const&,
                                   std::size_t rangeSetID);

  }
}

#endif

// Local variables:
// mode: c++
// End:
