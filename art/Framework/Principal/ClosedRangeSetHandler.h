#ifndef art_Framework_Principal_ClosedRangeSetHandler_h
#define art_Framework_Principal_ClosedRangeSetHandler_h

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <string>
#include <vector>

namespace art {

  class ClosedRangeSetHandler : public RangeSetHandler {
  public:

    explicit ClosedRangeSetHandler();
    explicit ClosedRangeSetHandler(RunNumber_t r);
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
    RangeSet const& do_getRanges() const override { return ranges_; }

    void do_updateFromEvent(EventID const&, bool lastInSubRun) override;
    void do_updateFromSubRun(SubRunID const&) override {}

    void do_flushRanges() override;
    void do_maybeSplitRange() override;
    void do_rebase() override;

    std::unique_ptr<RangeSetHandler> do_clone() const override
    {
      return std::make_unique<ClosedRangeSetHandler>(ranges_);
    }

    RangeSet ranges_;
    RangeSet::const_iterator rsIter_ {ranges_.begin()};
    EventID lastSeenEvent_ {EventID::invalidEvent()};
  };

}
#endif /* art_Utilities_ClosedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
