#include "art/Framework/IO/Root/detail/RangeSetInfo.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

art::detail::RangeSetInfo::RangeSetInfo(RunNumber_t const r,
                                        std::vector<EventRange>&& ers)
  : run{r}, ranges{std::move(ers)}
{}

bool
art::detail::RangeSetInfo::is_invalid() const
{
  return run == IDNumber<Level::Run>::invalid();
}

void
art::detail::RangeSetInfo::update(RangeSetInfo&& rsi, bool compact)
{
  if (run != rsi.run) {
    throw art::Exception(art::errors::LogicError)
      << "Cannot merge two ranges-of-validity with different run numbers: "
      << run << " vs. " << rsi.run << '\n'
      << "Please contact artists@fnal.gov.";
  }

  // If full range-set information is to be retained (the
  // default), then append the accumulated ranges into the
  // already-existing container.  Bail out early.
  if (!compact) {
    std::move(rsi.ranges.begin(), rsi.ranges.end(), std::back_inserter(ranges));
    return;
  }

  // For compact ranges
  for (auto&& range : rsi.ranges) {
    auto const subRunN = range.subRun();
    // If compact option is chosen, there will be *at most*
    // one entry in the 'ranges' container that corresponds to
    // this subrun.
    auto found =
      std::find_if(begin(ranges), end(ranges), [subRunN](auto const& er) {
        return er.subRun() == subRunN;
      });
    if (found == end(ranges)) {
      ranges.push_back(std::move(range));
      continue;
    }

    throw_if_not_disjoint(run, *found, range);
    auto const ebegin = std::min(found->begin(), range.begin());
    auto const eend = std::max(found->end(), range.end());
    *found = EventRange{subRunN, ebegin, eend};
  }
}
