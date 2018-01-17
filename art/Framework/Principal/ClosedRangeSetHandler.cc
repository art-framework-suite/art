#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>

namespace {
  constexpr auto invalid_eid[[gnu::unused]] =
    art::IDNumber<art::Level::Event>::invalid();
}

namespace art {

  ClosedRangeSetHandler::ClosedRangeSetHandler(RangeSet const& rs) : ranges_{rs}
  {}

  RangeSet
  ClosedRangeSetHandler::do_getSeenRanges() const
  {
    RangeSet tmp{ranges_.run()};
    tmp.assign_ranges(begin(), rsIter_);
    return tmp;
  }

  void
  ClosedRangeSetHandler::do_update(EventID const& id, bool const lastInSubRun)
  {
    eventInfo_.set(id, lastInSubRun);

    if (lastInSubRun) {
      rsIter_ = next_subrun_or_end();
      return;
    }

    while (rsIter_ != end() && !rsIter_->contains(id.subRun(), id.event())) {
      ++rsIter_;
    }
  }

  void
  ClosedRangeSetHandler::do_flushRanges()
  {
    rsIter_ = end();
  }

  void
  ClosedRangeSetHandler::do_maybeSplitRange()
  {
    if (!eventInfo_.lastInSubRun && rsIter_ != end()) {
      auto split_range = ranges_.split_range(eventInfo_.id.subRun(),
                                             eventInfo_.id.next().event());
      if (split_range.second)
        rsIter_ = split_range.first;
    }
  }

  void
  ClosedRangeSetHandler::do_rebase()
  {
    std::vector<EventRange> rebasedRanges(rsIter_, end());
    RangeSet tmpRS{ranges_.run(), rebasedRanges};
    ClosedRangeSetHandler tmp{tmpRS};
    std::swap(*this, tmp);
  }

  RangeSet::const_iterator
  ClosedRangeSetHandler::next_subrun_or_end() const
  {
    if (rsIter_ == end())
      return end();

    auto const sr = rsIter_->subRun();
    auto pos = std::find_if(
      rsIter_, end(), [sr](auto const& range) { return range.subRun() != sr; });
    return pos;
  }
}
