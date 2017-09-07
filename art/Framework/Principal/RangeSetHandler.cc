#include "art/Framework/Principal/RangeSetHandler.h"
// vim: set sw=2 expandtab :

namespace art {

RangeSetHandler::
~RangeSetHandler() noexcept
{
}

RangeSetHandler::HandlerType
RangeSetHandler::
type() const
{
  return do_type();
}

RangeSet
RangeSetHandler::
seenRanges() const
{
  return do_getSeenRanges();
}

void
RangeSetHandler::
update(EventID const& id, bool const lastInSubRun)
{
  do_update(id, lastInSubRun);
}

void
RangeSetHandler::
flushRanges()
{
  do_flushRanges();
}

void
RangeSetHandler::
maybeSplitRange()
{
  do_maybeSplitRange();
}

void
RangeSetHandler::
rebase()
{
  do_rebase();
}

RangeSetHandler*
RangeSetHandler::
clone() const
{
  return do_clone();
}

} // namespace art

