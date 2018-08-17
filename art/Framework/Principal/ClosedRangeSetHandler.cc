#include "art/Framework/Principal/ClosedRangeSetHandler.h"
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

  namespace {
    constexpr auto invalid_eid[[maybe_unused]] =
      IDNumber<Level::Event>::invalid();
  } // unnamed namespace

  ClosedRangeSetHandler::EventInfo::~EventInfo() noexcept = default;

  ClosedRangeSetHandler::EventInfo::EventInfo() noexcept = default;
  ClosedRangeSetHandler::EventInfo::EventInfo(EventInfo const& rhs) noexcept =
    default;

  ClosedRangeSetHandler::EventInfo::EventInfo(EventInfo&& rhs) noexcept =
    default;

  ClosedRangeSetHandler::EventInfo& ClosedRangeSetHandler::EventInfo::operator=(
    EventInfo const& rhs) noexcept = default;

  ClosedRangeSetHandler::EventInfo& ClosedRangeSetHandler::EventInfo::operator=(
    EventInfo&& rhs) noexcept = default;

  EventID const&
  ClosedRangeSetHandler::EventInfo::id() const
  {
    return id_;
  }

  bool
  ClosedRangeSetHandler::EventInfo::lastInSubRun() const
  {
    return lastInSubRun_;
  }

  void
  ClosedRangeSetHandler::EventInfo::set(EventID const& eid, bool const last)
  {
    id_ = eid;
    lastInSubRun_ = last;
  }

  // Note: RangeSet has a data member that is a vector, and the vector
  // dtor is not noexcept, so we cannot be noexcept either!
  ClosedRangeSetHandler::~ClosedRangeSetHandler() = default;

  // Note: RangeSet has a data member that is a vector, and the vector
  // ctor is not noexcept, so we cannot be noexcept either!
  ClosedRangeSetHandler::ClosedRangeSetHandler(RangeSet const& rs)
    : ranges_{rs}, idx_{0}, eventInfo_{}
  {}

  // Note: RangeSet has a data member that is a vector, and the vector
  // copy ctor is not noexcept, so we cannot be noexcept either!
  ClosedRangeSetHandler::ClosedRangeSetHandler(
    ClosedRangeSetHandler const& rhs) = default;

  // Note: RangeSet has a data member that is a vector, and a vector
  // move constructor is not noexcept, so we cannot be noexcept
  // either!
  ClosedRangeSetHandler::ClosedRangeSetHandler(ClosedRangeSetHandler&& rhs) =
    default;

  // Note: RangeSet has a data member that is a vector, and the vector
  // copy assignment is not noexcept, so we cannot be noexcept either!
  ClosedRangeSetHandler& ClosedRangeSetHandler::operator=(
    ClosedRangeSetHandler const& rhs) = default;

  // Note: RangeSet has a data member that is a vector, and a vector
  // move assignment is not noexcept, so we cannot be noexcept either!
  ClosedRangeSetHandler& ClosedRangeSetHandler::operator=(
    ClosedRangeSetHandler&& rhs) = default;

  ClosedRangeSetHandler::EventInfo const&
  ClosedRangeSetHandler::eventInfo() const
  {
    return eventInfo_;
  }

  size_t
  ClosedRangeSetHandler::begin_idx() const
  {
    return ranges_.begin_idx();
  }

  size_t
  ClosedRangeSetHandler::end_idx() const
  {
    return ranges_.end_idx();
  }

  RangeSetHandler::HandlerType
  ClosedRangeSetHandler::do_type() const
  {
    return HandlerType::Closed;
  }

  RangeSet
  ClosedRangeSetHandler::do_getSeenRanges() const
  {
    RangeSet tmp{ranges_.run()};
    tmp.assign_ranges(ranges_, begin_idx(), idx_);
    return tmp;
  }

  void
  ClosedRangeSetHandler::do_update(EventID const& id, bool const lastInSubRun)
  {
    eventInfo_.set(id, lastInSubRun);
    if (lastInSubRun) {
      idx_ = ranges_.next_subrun_or_end(idx_);
      return;
    }
    while (idx_ != end_idx() &&
           !ranges_.at(idx_).contains(id.subRun(), id.event())) {
      ++idx_;
    }
  }

  void
  ClosedRangeSetHandler::do_flushRanges()
  {
    idx_ = end_idx();
  }

  void
  ClosedRangeSetHandler::do_maybeSplitRange()
  {
    if (eventInfo_.lastInSubRun()) {
      // No need to split.
      return;
    }
    if (!eventInfo_.id().isValid()) {
      // Have not read any events yet.
      return;
    }
    if (eventInfo_.id().isFlush()) {
      // Should not happen, be careful anyway.
      return;
    }
    if (idx_ == end_idx()) {
      // No need to split.
      return;
    }
    auto split_range = ranges_.split_range(eventInfo_.id().subRun(),
                                           eventInfo_.id().next().event());
    if (split_range.second) {
      // The split did happen, update our idx to the right-hand
      // side of the split (our ranges has already been updated).
      idx_ = split_range.first;
    }
  }

  void
  ClosedRangeSetHandler::do_rebase()
  {
    auto rebasedRanges = ranges_.extract_ranges(idx_, end_idx());
    RangeSet tmpRS{ranges_.run(), rebasedRanges};
    ClosedRangeSetHandler tmp{tmpRS};
    std::swap(*this, tmp);
  }

  RangeSetHandler*
  ClosedRangeSetHandler::do_clone() const
  {
    return new ClosedRangeSetHandler{*this};
  }

} // namespace art
