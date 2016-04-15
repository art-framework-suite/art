#ifndef art_Framework_Principal_RangeSetHandler_h
#define art_Framework_Principal_RangeSetHandler_h

#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {

  class EventID;
  class SubRunID;

  namespace detail {

    class RangeSetHandler {
    public:

      RangeSet seenRanges() const { return do_getSeenRanges(); }

      void updateFromEvent(EventID const& id, bool const lastInSubRun)
      {
        do_updateFromEvent(id, lastInSubRun);
      }

      void updateFromSubRun(SubRunID const& id)
      {
        do_updateFromSubRun(id);
      }

      void flushRanges() { do_flushRanges(); }
      void maybeSplitRange() { do_maybeSplitRange(); }
      void rebase() { do_rebase(); }

    private:
      virtual RangeSet do_getSeenRanges() const = 0;

      virtual void do_updateFromEvent(EventID const&, bool lastInSubRun) = 0;
      virtual void do_updateFromSubRun(SubRunID const&) = 0;
      virtual void do_flushRanges() = 0;
      virtual void do_maybeSplitRange() = 0;
      virtual void do_rebase() = 0;
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
