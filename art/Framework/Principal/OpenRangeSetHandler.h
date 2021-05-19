#ifndef art_Framework_Principal_OpenRangeSetHandler_h
#define art_Framework_Principal_OpenRangeSetHandler_h
// vim: set sw=2 expandtab :

//
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
//      "rebased" (or cleared for an OpenRangeSetHandler).  The update
//      function will appropriately assign the RangeHandler to the
//      correct IDs when the next event is read.  For example, if
//      EmptyEvent has been configured to produce 20 events for Run 1,
//      SubRun 0, and there is an output-file switch after event 10,
//      the RangeSets for the two files will be:
//
//        File A RangeSet = Run: 1 SubRun: 0 Event range: [1,11)
//        File B RangeSet = Run: 1 SubRun: 0 Event range: [11,20)
//
//      regardless of any events that may have failed a filter.
//

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {

  class OpenRangeSetHandler final : public RangeSetHandler {
  public:
    virtual ~OpenRangeSetHandler();

    explicit OpenRangeSetHandler(RunNumber_t r);
    explicit OpenRangeSetHandler(RangeSet const&);

    OpenRangeSetHandler(OpenRangeSetHandler const&);
    OpenRangeSetHandler(OpenRangeSetHandler&&);

    OpenRangeSetHandler& operator=(OpenRangeSetHandler const&);
    OpenRangeSetHandler& operator=(OpenRangeSetHandler&&);

  private:
    HandlerType do_type() const override;
    RangeSet do_getSeenRanges() const override;

    void do_update(EventID const&, bool lastInSubRun) override;
    void do_flushRanges() override;
    void do_maybeSplitRange() override;
    void do_rebase() override;
    RangeSetHandler* do_clone() const override;

    RangeSet ranges_{RangeSet::invalid()};
    std::size_t idx_{0};
  };

} // namespace art

#endif /* art_Framework_Principal_OpenRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
