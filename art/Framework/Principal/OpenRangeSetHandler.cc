#include "art/Framework/Principal/OpenRangeSetHandler.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace art {

// Note: RangeSet has a data member that is
// a vector, and the vector dtor is not
// noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler::
~OpenRangeSetHandler()
{
}

// Note: RangeSet has a data member that is
// a vector, and the vector ctor is not
// noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler::
OpenRangeSetHandler(RunNumber_t const r)
  : ranges_{r}
  , idx_{0}
{
}

// Note: RangeSet has a data member that is
// a vector, and the vector ctor is not
// noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler::
OpenRangeSetHandler(RangeSet const& ranges)
  : ranges_{ranges}
  , idx_{ranges.end_idx()}
{
}

// Note: RangeSet has a data member that is
// a vector, and the vector copy ctor is not
// noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler::
OpenRangeSetHandler(OpenRangeSetHandler const& rhs)
  : ranges_{rhs.ranges_}
  , idx_{rhs.idx_}
{
}

// Note: RangeSet has a data member that is
// a vector, and a vector move constructor
// is not noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler::
OpenRangeSetHandler(OpenRangeSetHandler&& rhs)
  : ranges_{std::move(rhs.ranges_)}
  , idx_{std::move(rhs.idx_)}
{
}

// Note: RangeSet has a data member that is
// a vector, and the vector copy assignment
// is not noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler&
OpenRangeSetHandler::
operator=(OpenRangeSetHandler const& rhs)
{
  if (this != &rhs) {
    ranges_ = rhs.ranges_;
    idx_ = rhs.idx_;
  }
  return *this;
}

// Note: RangeSet has a data member that is
// a vector, and a vector move assignment
// is not noexcept, so we cannot be noexcept
// either!
OpenRangeSetHandler&
OpenRangeSetHandler::
operator=(OpenRangeSetHandler&& rhs)
{
  ranges_ = std::move(rhs.ranges_);
  idx_ = std::move(rhs.idx_);
  return *this;
}

RangeSetHandler::HandlerType
OpenRangeSetHandler::
do_type() const
{
  return HandlerType::Open;
}

RangeSet
OpenRangeSetHandler::
do_getSeenRanges() const
{
  RangeSet tmp{ranges_.run()};
  tmp.assign_ranges(ranges_, ranges_.begin_idx(), idx_);
  return tmp;
}

void
OpenRangeSetHandler::
do_update(EventID const& id, bool const /*lastInSubRun*/)
{
  if (ranges_.empty()) {
    ranges_.set_run(id.run());
    ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
    idx_ = ranges_.end_idx();
    return;
  }
  auto& back = ranges_.back();
  if ((back.subRun() == id.subRun()) && (back.end() == id.event())) {
    back.set_end(id.next().event());
    return;
  }
  ranges_.emplace_range(id.subRun(), id.event(), id.next().event());
  idx_ = ranges_.end_idx();
}

void
OpenRangeSetHandler::
do_flushRanges()
{
}

void
OpenRangeSetHandler::
do_maybeSplitRange()
{
}

void
OpenRangeSetHandler::
do_rebase()
{
  ranges_.clear();
  idx_ = ranges_.end_idx();
}

RangeSetHandler*
OpenRangeSetHandler::
do_clone() const
{
  return new OpenRangeSetHandler{*this};
}

} // namespace art

