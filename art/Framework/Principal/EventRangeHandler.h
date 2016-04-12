#ifndef art_Framework_Principal_EventRangeHandler_h
#define art_Framework_Principal_EventRangeHandler_h

// EventRangeHandler is used by the SubRunPrincipal to:
//
//   - Accept a vector of EventRanges from an input file (if present).
//   - Combine mergeable ranges from the input file.
//   - Create sliding output ranges

#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {

  class EventID;

  class EventRangeHandler {
  public:

    explicit EventRangeHandler();
    explicit EventRangeHandler(RunNumber_t r);
    explicit EventRangeHandler(RangeSet const& inputRangeSet);

    void update(EventID const& id, bool lastEventOfSubRun);
    void update(SubRunID const& id);
    void initializeRanges(RangeSet const& inputRangeSet);
    void setOutputRanges(RangeSet const& outputRangeSet);
    void setOutputRanges(RangeSet::const_iterator b,
                         RangeSet::const_iterator e);
    void rebase();
    void reset();

    auto const& inputRanges() const { return inputRanges_; }
    auto const& outputRanges() const { return outputRanges_; }
    auto begin() const { return outputRanges_.ranges().begin(); }
    auto current() const { return rsIter_ == end() ? end() : rsIter_+1; }

  private:
    RangeSet::const_iterator end() const { return outputRanges_.ranges().end(); }
    RangeSet inputRanges_;
    RangeSet outputRanges_;
    RangeSet::const_iterator rsIter_ {outputRanges_.ranges().begin()};
    bool lastEventOfSubRunSeen_ {false};
  };

}
#endif /* art_Utilities_EventRangeHandler_h */

// Local Variables:
// mode: c++
// End:
