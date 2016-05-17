#ifndef art_Framework_Principal_ClosedRangeSetHandler_h
#define art_Framework_Principal_ClosedRangeSetHandler_h

// ====================================================================
// ClosedRangeSetHandler is used to track and manipulate RangeSets as
// inherited from another source, such as RootInput.  The RangeSets
// are closed in that the span of events/subruns encapsulated by a
// given RangeSet can never grow.  The individual ranges can be split
// if an output-file switch necessitates.  The 'rsIter_' member keeps
// track of the current EventRange.
// ====================================================================

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <string>
#include <vector>

namespace art {

  class ClosedRangeSetHandler : public RangeSetHandler {
  public:

    explicit ClosedRangeSetHandler(RangeSet const& inputRangeSet);

    // This class contains an iterator as a member.
    // It should not be copied!
    ClosedRangeSetHandler(ClosedRangeSetHandler const&) = delete;
    ClosedRangeSetHandler& operator=(ClosedRangeSetHandler const&) = delete;

    ClosedRangeSetHandler(ClosedRangeSetHandler&&) = default;
    ClosedRangeSetHandler& operator=(ClosedRangeSetHandler&&) = default;

  private:

    auto begin() const { return ranges_.begin(); }
    auto end() const { return ranges_.end(); }
    RangeSet::const_iterator next_subrun_or_end() const;

    RangeSet do_getSeenRanges() const override;

    void do_update(EventID const&, bool lastInSubRun) override;

    void do_flushRanges() override;
    void do_maybeSplitRange() override;
    void do_rebase() override;

    struct EventInfo {
      void set(EventID const& eid, bool const last)
      {
        id = eid;
        lastInSubRun = last;
      }
      EventID id {EventID::invalidEvent()};
      bool lastInSubRun {false};
    };

    RangeSet ranges_ {RangeSet::invalid()};
    RangeSet::const_iterator rsIter_ {ranges_.begin()};
    EventInfo eventInfo_ {};
  };

}
#endif /* art_Utilities_ClosedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
