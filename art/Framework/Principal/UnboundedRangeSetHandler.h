#ifndef art_Framework_Principal_UnboundedRangeSetHandler_h
#define art_Framework_Principal_UnboundedRangeSetHandler_h

// UnboundedRangeSetHandler is used by the SubRunPrincipal to:
//
//   - Accept a vector of EventRanges from an input file (if present).
//   - Combine mergeable ranges from the input file.
//   - Create sliding output ranges

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {

  class UnboundedRangeSetHandler : public detail::RangeSetHandler {
  public:

    explicit UnboundedRangeSetHandler();
    explicit UnboundedRangeSetHandler(RunNumber_t r);
    explicit UnboundedRangeSetHandler(RangeSet const& inputRangeSet);

    // This class contains an iterator as a member.
    // It should not be copied!
    UnboundedRangeSetHandler(UnboundedRangeSetHandler const&) = delete;
    UnboundedRangeSetHandler& operator=(UnboundedRangeSetHandler const&) = delete;

    UnboundedRangeSetHandler(UnboundedRangeSetHandler&&) = default;
    UnboundedRangeSetHandler& operator=(UnboundedRangeSetHandler&&) = default;

  private:

    auto begin() const { return ranges_.begin(); }
    auto end() const { return ranges_.end(); }

    RangeSet do_getSeenRanges() const override;

    void do_updateFromEvent(EventID const&, bool lastInSubRun) override;
    void do_updateFromSubRun(SubRunID const&) override;
    void do_flushRanges() override {}
    void do_maybeSplitRange() override {}
    void do_rebase() override;

    RangeSet ranges_;
    RangeSet::const_iterator rsIter_ {ranges_.begin()};
    bool lastInSubRun_ {true};
  };

}
#endif /* art_Utilities_UnboundedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
