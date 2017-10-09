#ifndef art_Framework_Principal_RangeSetHandler_h
#define art_Framework_Principal_RangeSetHandler_h

#include "canvas/Persistency/Provenance/RangeSet.h"
#include <memory>

namespace art {

  class EventID;
  class SubRunID;

  class RangeSetHandler {
  public:
    virtual ~RangeSetHandler() noexcept = default;

    RangeSet
    seenRanges() const
    {
      return do_getSeenRanges();
    }

    void
    update(EventID const& id, bool const lastInSubRun)
    {
      do_update(id, lastInSubRun);
    }

    void
    flushRanges()
    {
      do_flushRanges();
    }
    void
    maybeSplitRange()
    {
      do_maybeSplitRange();
    }
    void
    rebase()
    {
      do_rebase();
    }

  private:
    virtual RangeSet do_getSeenRanges() const = 0;

    virtual void do_update(EventID const&, bool lastInSubRun) = 0;
    virtual void do_flushRanges() = 0;
    virtual void do_maybeSplitRange() = 0;
    virtual void do_rebase() = 0;
  };
}

#endif /* art_Framework_Principal_RangeSetHandler_h */

// Local variables:
// mode: c++
// End:
