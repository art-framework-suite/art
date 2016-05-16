#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>

namespace {
  constexpr auto invalid_eid = art::IDNumber<art::Level::Event>::invalid();
}

namespace art {

  OpenRangeSetHandler::OpenRangeSetHandler(RunNumber_t const r)
    : ranges_{r}
  {}

  RangeSet
  OpenRangeSetHandler::do_getSeenRanges() const
  {
    RangeSet tmp {ranges_.run()};
    tmp.assign_ranges(begin(), rsIter_);
    return tmp;
  }

  void
  OpenRangeSetHandler::do_update(EventID const& id, bool const lastInSubRun)
  {
    lastInSubRun_ = lastInSubRun;

    if (ranges_.empty()) {
      ranges_.set_run(id.run());
      ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
      rsIter_ = ranges_.end();
      return;
    }
    auto& back = ranges_.back();
    if (back.subRun() == id.subRun()) {
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
  OpenRangeSetHandler::do_rebase()
  {
    if (ranges_.empty())
      return;

    auto const back = ranges_.back();
    ranges_.clear();
    rsIter_ = ranges_.end();

    // If last event in SubRun has been seen, do not rebase since a
    // new RangeSetHandler will be created for the next SubRun.
    if (lastInSubRun_)
      return;

    if (back.is_valid() && !back.is_full_subRun()) {
      ranges_.emplace_range(back.subRun(), back.end(), IDNumber<Level::Event>::next(back.end()));
      rsIter_ = ranges_.end();
    }
  }

}
