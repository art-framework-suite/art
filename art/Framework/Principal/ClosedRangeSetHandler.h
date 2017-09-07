#ifndef art_Framework_Principal_ClosedRangeSetHandler_h
#define art_Framework_Principal_ClosedRangeSetHandler_h
// vim: set sw=2 expandtab :

//
// ClosedRangeSetHandler
//
// This class is used to track AND MANIPULATE RangeSets as inherited
// from another source, such as RootInput.  The RangeSets are closed
// in that the span of events/subruns encapsulated by a given RangeSet
// can never grow.  The individual ranges, however, can be split if
// necessitated by an output-file switch.  The 'idx_' member keeps
// track of the current EventRange.
//
// N.B. Event-filtering does not affect the calculation of the
//      RangeSet since the RangeSet tracks all processed events, even
//      those that were rejected due to failing a filter criterion.
//
//      In the case of an output-file-switch, one of the RangeSet's
//      EventRanges might be "split" so as to ensure unique RangeSets
//      per output file for a given output module.  For example,
//      suppose the inherited RangeSet looks like:
//
//         Run: 1 SubRun: 0 Event range: [1,6)
//
//      but there are only two events in the file for that RangeSet --
//      e.g. 2 and 4.  If there is an output file switch after event 2
//      is processed, the RangeSets for the output files will be:
//
//         File A RangeSet = Run: 1 SubRun: 0 Event range: [1,3)
//         File B RangeSet = Run: 1 SubRun: 0 Event range: [3,6)
//
//      even though file A will contain only event 2 and file B will
//      contain only event 4.  In this way, the inherited RangeSet is
//      preserved across files.
//

#include "art/Framework/Principal/RangeSetHandler.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <string>
#include <vector>

namespace art {

class ClosedRangeSetHandler final : public RangeSetHandler {

public: // TYPES

  class EventInfo {

  public: // MEMBER FUNCTIONS -- Special Member Functions

    ~EventInfo() noexcept;

    EventInfo() noexcept;

    EventInfo(EventInfo const&) noexcept;

    EventInfo(EventInfo&&) noexcept;

    ClosedRangeSetHandler::EventInfo&
    operator=(EventInfo const&) noexcept;

    ClosedRangeSetHandler::EventInfo&
    operator=(EventInfo&&) noexcept;

  public: // MEMBER FUNCTIONS -- API for the user

    EventID const&
    id() const;

    bool
    lastInSubRun() const;

    void
    set(EventID const& eid, bool const last);

  private: // MEMBER DATA

    EventID
    id_{EventID::invalidEvent()};

    bool
    lastInSubRun_{false};

  };

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~ClosedRangeSetHandler();

  explicit
  ClosedRangeSetHandler(RangeSet const& inputRangeSet);

  ClosedRangeSetHandler(ClosedRangeSetHandler const&);

  ClosedRangeSetHandler(ClosedRangeSetHandler&&);

  ClosedRangeSetHandler&
  operator=(ClosedRangeSetHandler const&);

  ClosedRangeSetHandler&
  operator=(ClosedRangeSetHandler&&);

public: // MEMBER FUNCTIONS -- API for the user

  EventInfo const&
  eventInfo() const;
  
private: // MEMBER FUNCTIONS -- Implementation details

  std::size_t
  begin_idx() const;

  std::size_t
  end_idx() const;

private: // MEMBER FUNCTIONS -- API required by RangeSetHandler

  HandlerType
  do_type() const override;

  RangeSet
  do_getSeenRanges() const override;

  void
  do_update(EventID const&, bool lastInSubRun) override;

  void
  do_flushRanges() override;

  void
  do_maybeSplitRange() override;

  void
  do_rebase() override;

  RangeSetHandler*
  do_clone() const override;

private: // MEMBER DATA

  RangeSet
  ranges_{RangeSet::invalid()};

  std::size_t
  idx_{0};

  EventInfo
  eventInfo_{};

};

} // namespace art

#endif /* art_Framework_Principal_ClosedRangeSetHandler_h */

// Local Variables:
// mode: c++
// End:
