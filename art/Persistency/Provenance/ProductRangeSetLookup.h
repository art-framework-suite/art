#ifndef art_Persistency_Provenance_ProductRangeSetLookup_h
#define art_Persistency_Provenance_ProductRangeSetLookup_h

#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <cassert>
#include <map>

namespace art {

  class ProductRangeSetLookup {
  public:

    void emplace(BranchID const bid, RangeSet const& rs) {
      auto const checksum = rs.checksum();
      rangeSets_.emplace(checksum, rs);
      productRangeSetChecksums_.emplace(bid, checksum);
    }

    void emplace(BranchKey const& bk, RangeSet&& rs) {
      auto to_bid = [](art::BranchKey const& bk) {
        return art::BranchID{bk.branchName()};
      };

      auto const checksum = rs.checksum();
      rangeSets_.emplace(checksum, std::move(rs));
      productRangeSetChecksums_.emplace(to_bid(bk), checksum);
    }

    RangeSet const* getRangeSet(BranchID const bid) const
    {
      auto it = productRangeSetChecksums_.find(bid);
      if (it == productRangeSetChecksums_.cend())
        return nullptr;

      auto const checksum = it->second;
      auto f = rangeSets_.find(checksum);
      assert(f != rangeSets_.cend()); // Based on construction, this must be true.
      return &f->second;
    }

  private:
    using RS_checksum_t = unsigned;
    std::map<RS_checksum_t, RangeSet> rangeSets_;
    std::map<BranchID,RS_checksum_t> productRangeSetChecksums_;
  };

}

#endif

// Local variables:
// mode: c++
// End:
