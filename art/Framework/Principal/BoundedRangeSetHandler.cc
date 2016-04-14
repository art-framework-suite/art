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
    : BoundedRangeSetHandler{RangeSet::forRun(r)}
  {}

  BoundedRangeSetHandler::BoundedRangeSetHandler(RangeSet const& rs)
    : ranges_{rs}
  {}

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
  BoundedRangeSetHandler::do_rebase()
  {
    assert(!ranges_.empty());
    auto const first = ranges_.split_range(lastSeenEvent_.subRun(),
                                           lastSeenEvent_.event());
    std::vector<EventRange> rebasedRanges (first, ranges_.end());
    RangeSet tmpRS {ranges_.run(), rebasedRanges};
    BoundedRangeSetHandler tmp {tmpRS};
    std::swap(*this, tmp);
  }

  void
  BoundedRangeSetHandler::do_reset()
  {
    BoundedRangeSetHandler tmp {IDNumber<Level::Run>::invalid()};
    std::swap(*this, tmp);
  }

}
