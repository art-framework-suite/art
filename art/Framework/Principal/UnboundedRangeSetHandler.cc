#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/UnboundedRangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>
#include <iostream>

namespace {
  constexpr auto invalid_eid = art::IDNumber<art::Level::Event>::invalid();
}

namespace art {

  UnboundedRangeSetHandler::UnboundedRangeSetHandler()
    : UnboundedRangeSetHandler{RangeSet::invalid()}
  {}

  UnboundedRangeSetHandler::UnboundedRangeSetHandler(RunNumber_t const r)
    : UnboundedRangeSetHandler{RangeSet::forRun(r)}
  {}

  UnboundedRangeSetHandler::UnboundedRangeSetHandler(RangeSet const& rs)
    : ranges_{rs}
  {}

  void
  UnboundedRangeSetHandler::do_updateFromEvent(EventID const& id,
                                               bool const /*lastInSubRun*/)
  {
    if (ranges_.empty()) {
      ranges_.set_run(id.run());
      ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
      rsIter_ = ranges_.end();
      return;
    }
    auto& back = ranges_.back();
    if (back.subrun() == id.subRun()) {
      if (back.end() == id.event()) {
        back.set_end(id.next().event());
      }
    }
    else {
      ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
      rsIter_ = ranges_.end();
    }
  }

  void
  UnboundedRangeSetHandler::do_updateFromSubRun(SubRunID const& id)
  {
    auto const r = id.run();
    auto const sr = id.subRun();
    if (ranges_.empty()) {
      ranges_.set_run(r);
      ranges_.emplace_range(sr, invalid_eid, invalid_eid);
      rsIter_ = ranges_.begin();
    }
    else if (ranges_.back().subrun() != sr) {
      ranges_.emplace_range(sr, invalid_eid, invalid_eid);
      rsIter_ = ranges_.begin();
    }
  }

  void
  UnboundedRangeSetHandler::do_rebase()
  {
    if (ranges_.empty())
      return;

    auto const back = ranges_.back();
    ranges_.clear();
    rsIter_ = ranges_.end();

    if (is_valid(back.subrun()) &&
        is_valid(back.begin()) &&
        is_valid(back.end())) {
      ranges_.emplace_range(back.subrun(), back.end(), IDNumber<Level::Event>::next(back.end()));
      rsIter_ = ranges_.end();
    }

  }

  void
  UnboundedRangeSetHandler::do_reset()
  {
    UnboundedRangeSetHandler tmp {IDNumber<Level::Run>::invalid()};
    std::swap(*this, tmp);
  }

}
