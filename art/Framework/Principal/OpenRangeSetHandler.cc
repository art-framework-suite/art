#include "art/Framework/Principal/OpenRangeSetHandler.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

#include <utility>

namespace art {

  OpenRangeSetHandler::~OpenRangeSetHandler() = default;

  // Note: RangeSet has a data member that is a vector, and the vector
  //       ctor is not noexcept, so we cannot be noexcept either!
  OpenRangeSetHandler::OpenRangeSetHandler(RunNumber_t const r) : ranges_{r} {}

  // Note: RangeSet has a data member that is a vector, and the vector
  //       ctor is not noexcept, so we cannot be noexcept either!
  OpenRangeSetHandler::OpenRangeSetHandler(RangeSet const& ranges)
    : ranges_{ranges}, idx_{ranges.end_idx()}
  {}

  OpenRangeSetHandler::OpenRangeSetHandler(OpenRangeSetHandler const&) =
    default;

  OpenRangeSetHandler::OpenRangeSetHandler(OpenRangeSetHandler&&) = default;

  // Note: RangeSet has a data member that is a vector, and the vector
  //       copy assignment is not noexcept, so we cannot be noexcept
  //       either!
  OpenRangeSetHandler& OpenRangeSetHandler::operator=(
    OpenRangeSetHandler const&) = default;

  // Note: RangeSet has a data member that is a vector, and a vector
  //       move assignment is not noexcept, so we cannot be noexcept
  //       either!
  OpenRangeSetHandler& OpenRangeSetHandler::operator=(
    OpenRangeSetHandler&& rhs) = default;

  RangeSetHandler::HandlerType
  OpenRangeSetHandler::do_type() const
  {
    return HandlerType::Open;
  }

  RangeSet
  OpenRangeSetHandler::do_getSeenRanges() const
  {
    RangeSet tmp{ranges_.run()};
    tmp.assign_ranges(ranges_, ranges_.begin_idx(), idx_);
    return tmp;
  }

  void
  OpenRangeSetHandler::do_update(EventID const& id, bool const /*lastInSubRun*/)
  {
    ranges_.update(id);
    idx_ = ranges_.end_idx();
  }

  void
  OpenRangeSetHandler::do_flushRanges()
  {}

  void
  OpenRangeSetHandler::do_maybeSplitRange()
  {}

  void
  OpenRangeSetHandler::do_rebase()
  {
    ranges_.clear();
    idx_ = ranges_.end_idx();
  }

  RangeSetHandler*
  OpenRangeSetHandler::do_clone() const
  {
    return new OpenRangeSetHandler{*this};
  }

} // namespace art
