#include "art/Framework/Principal/BoundedRangeSetHandler.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>
#include <iostream>

namespace {
  constexpr auto invalid_eid = art::IDNumber<art::Level::Event>::invalid();
}

namespace art {

  BoundedRangeSetHandler::BoundedRangeSetHandler()
    : BoundedRangeSetHandler{RangeSet::invalid()}
  {}

  BoundedRangeSetHandler::BoundedRangeSetHandler(RunNumber_t const r)
    : BoundedRangeSetHandler{RangeSet{r}}
  {}

  BoundedRangeSetHandler::BoundedRangeSetHandler(RangeSet const& rs)
    : ranges_{rs}
  {}

  RangeSet
  BoundedRangeSetHandler::do_getSeenRanges() const {
    RangeSet tmp {ranges_.run()};
    tmp.assign_ranges(begin(), rsIter_);
    return tmp;
  }

  void
  BoundedRangeSetHandler::do_updateFromEvent(EventID const& id,
                                             bool const lastInSubRun)
  {
    lastSeenEvent_ = id;

    if (lastInSubRun) {
      rsIter_ = end();
      return;
    }

    while (rsIter_ != end() && !rsIter_->contains(id.subRun(), id.event())) {
      ++rsIter_;
    }
  }

  void
  BoundedRangeSetHandler::do_flushRanges()
  {
    rsIter_ = end();
  }

  void
  BoundedRangeSetHandler::do_maybeSplitRange()
  {
    if (rsIter_ != end()) {
      rsIter_ = ranges_.split_range(lastSeenEvent_.subRun(),
                                    lastSeenEvent_.event());
    }
  }

  void
  BoundedRangeSetHandler::do_rebase()
  {
    std::vector<EventRange> rebasedRanges (rsIter_, end());
    RangeSet tmpRS {ranges_.run(), rebasedRanges};
    BoundedRangeSetHandler tmp {tmpRS};
    std::swap(*this, tmp);
  }

}
