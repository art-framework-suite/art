#ifndef art_Framework_Principal_BoundedRangeSetHandler_h
#define art_Framework_Principal_BoundedRangeSetHandler_h

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {

  class BoundedRangeSetHandler : public detail::RangeSetHandler {
  public:

    explicit BoundedRangeSetHandler();
    explicit BoundedRangeSetHandler(RunNumber_t r);
    explicit BoundedRangeSetHandler(RangeSet const& inputRangeSet);

    // This class contains an iterator as a member.
    // It should not be copied!
    BoundedRangeSetHandler(BoundedRangeSetHandler const&) = delete;
    BoundedRangeSetHandler& operator=(BoundedRangeSetHandler const&) = delete;

    BoundedRangeSetHandler(BoundedRangeSetHandler&&) = default;
    BoundedRangeSetHandler& operator=(BoundedRangeSetHandler&&) = default;

  private:

    auto begin() const { return ranges_.begin(); }
    auto end() const { return ranges_.end(); }

    RangeSet do_getSeenRanges() const override;

    void do_updateFromEvent(EventID const&, bool lastInSubRun) override;
    void do_updateFromSubRun(SubRunID const&) override {}

    void do_flushRanges() override;
    void do_maybeSplitRange() override;
    void do_rebase() override;

    RangeSet ranges_;
    RangeSet::const_iterator rsIter_ {ranges_.begin()};
    EventID lastSeenEvent_ {EventID::invalidEvent()};
  };

}
#endif /* art_Utilities_BoundedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
