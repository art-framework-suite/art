#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>

namespace art {

  OpenRangeSetHandler::OpenRangeSetHandler(RunNumber_t const r)
    : ranges_{r}
  {}

  RangeSet
  OpenRangeSetHandler::do_getSeenRanges() const
  {
    RangeSet tmp {ranges_.run()};
    tmp.assign_ranges(ranges_.begin(), rsIter_);
    return tmp;
  }

  void
  OpenRangeSetHandler::do_update(EventID const& id, bool const /*lastInSubRun*/)
  {
    if (ranges_.empty()) {
      ranges_.set_run(id.run());
      ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
      rsIter_ = ranges_.end();
      return;
    }
    auto& back = ranges_.back();
    if (back.subRun() == id.subRun() && back.end() == id.event()) {
      back.set_end(id.next().event());
    }
    else {
      ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
      rsIter_ = ranges_.end();
    }
  }

  void
  OpenRangeSetHandler::do_rebase()
  {
    ranges_.clear();
    rsIter_ = ranges_.end();
  }

}
