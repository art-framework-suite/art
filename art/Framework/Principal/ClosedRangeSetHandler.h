#ifndef art_Framework_Principal_ClosedRangeSetHandler_h
#define art_Framework_Principal_ClosedRangeSetHandler_h

// ====================================================================
// ClosedRangeSetHandler
//
// This class is used to track AND MANIPULATE RangeSets as inherited
// from another source, such as RootInput.  The RangeSets are closed
// in that the span of events/subruns encapsulated by a given RangeSet
// can never grow.  The individual ranges, however, can be split if
// necessitated by an output-file switch.  The 'rsIter_' member keeps
// track of the current EventRange.
//
// N.B. Event-filtering does not affect the calculation of the
//      RangeSet since the RangeSet tracks all processed events, even
//      those that were rejected due to failing a filter criterion.
//
//      In the case of an output-file-switch, one of the RangeSet's
//      EventRanges might be "split" so as to ensure unique RangeSets
//      per output file for a given output module.  For example,
//      suppose the inherited RangeSet looks like:
//
//         Run: 1 SubRun: 0 Event range: [1,6)
//
//      but there are only two events in the file for that RangeSet --
//      e.g. 2 and 4.  If there is an output file switch after event 2
//      is processed, the RangeSets for the output files will be:
//
//         File A RangeSet = Run: 1 SubRun: 0 Event range: [1,3)
//         File B RangeSet = Run: 1 SubRun: 0 Event range: [3,6)
//
//      even though file A will contain only event 2 and file B will
//      contain only event 4.  In this way, the inherited RangeSet is
//      preserved across files.
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
#endif /* art_Framework_Principal_ClosedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
