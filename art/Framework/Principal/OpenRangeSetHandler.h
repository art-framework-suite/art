#ifndef art_Framework_Principal_OpenRangeSetHandler_h
#define art_Framework_Principal_OpenRangeSetHandler_h

// ===================================================================
// OpenRangeSetHandler
//
// This class is used to track the in-memory RangeSet for processes
// whose sources are EmptyEvent or similar.  The RangeSet is only
// seeded with a run number, and the RangeSet grows whenever a new
// event is processed.  In other words, an OpenRangeSetHandler
// instance does not inherit any RangeSet from another source; it
// starts from scratch.
//
// N.B. Event-filtering does not affect the calculation of the
//      RangeSet since the RangeSet tracks all processed events, even
//      those that were rejected due to failing a filter criterion.
//
//      In the case of an output-file switch, the RangeSet is
//      "rebased" after the file-write, so that the updated RangeSet
//      now begins where the non-updated RangeSet finished.  For
//      example, if EmptyEvent has been configured to produce 20
//      events for Run 1, SubRun 0, and there is an output-file switch
//      after event 10, the RangeSets for the two files will be:
//
//        File A RangeSet = Run: 1 SubRun: 0 Event range: [1,11)
//        File B RangeSet = Run: 1 SubRun: 0 Event range: [11,20)
//
//      regardless of any events that may have failed a filter.
// ===================================================================

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <string>
#include <vector>

namespace art {

  class OpenRangeSetHandler : public RangeSetHandler {
  public:

    explicit OpenRangeSetHandler(RunNumber_t r);

    // This class contains an iterator as a member.
    // It should not be copied!
    OpenRangeSetHandler(OpenRangeSetHandler const&) = delete;
    OpenRangeSetHandler& operator=(OpenRangeSetHandler const&) = delete;

    OpenRangeSetHandler(OpenRangeSetHandler&&) = default;
    OpenRangeSetHandler& operator=(OpenRangeSetHandler&&) = default;

  private:

    auto begin() const { return ranges_.begin(); }
    auto end() const { return ranges_.end(); }

    RangeSet do_getSeenRanges() const override;

    void do_update(EventID const&, bool lastInSubRun) override;
    void do_flushRanges() override {}
    void do_maybeSplitRange() override {}
    void do_rebase() override;

    RangeSet ranges_ {RangeSet::invalid()};
    RangeSet::const_iterator rsIter_ {ranges_.begin()};
    bool lastInSubRun_ {true};
  };

}
#endif /* art_Utilities_OpenRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
