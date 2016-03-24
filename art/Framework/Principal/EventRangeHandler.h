#ifndef art_Framework_Principal_EventRangeHandler_h
#define art_Framework_Principal_EventRangeHandler_h

// EventRangeHandler is used by the SubRunPrincipal to:
//
//   - Accept a vector of EventRanges from an input file (if present).
//   - Combine mergeable ranges from the input file.
//   - Create sliding output ranges

#include "canvas/Persistency/Provenance/RangeSet.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {

  class EventID;

  class EventRangeHandler {
  public:

    explicit EventRangeHandler(RunNumber_t r);
    explicit EventRangeHandler(RangeSet const& inputRangeSet);

    void update(EventID const& id, bool lastEventOfSubRun);
    void setOutputRanges(RangeSet const& outputRangeSet);
    void rebase();

    auto const& inputRanges() const { return inputRanges_; }
    auto const& outputRanges() const { return outputRanges_; }

  private:
    RangeSet inputRanges_;
    RangeSet outputRanges_;
    bool lastEventOfSubRunSeen_ {false};
  };

}
#endif /* art_Utilities_EventRangeHandler_h */

// Local Variables:
// mode: c++
// End:
