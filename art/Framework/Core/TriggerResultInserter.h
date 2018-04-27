#ifndef art_Framework_Core_TriggerResultInserter_h
#define art_Framework_Core_TriggerResultInserter_h
// vim: set sw=2 expandtab :

// ======================================================================
// This is an unusual module in that it is always present in the
// schedule and it is not configurable.  The ownership of the bitmask
// is shared with the scheduler.  Its purpose is to create a
// TriggerResults instance and insert it into the event.
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <vector>

namespace art {
  class Event;
  class HLTGlobalStatus;
  class TriggerResultInserter : public ReplicatedProducer {
  public:
    // the pset needed here is the one that defines the trigger path names
    explicit TriggerResultInserter(fhicl::ParameterSet const&,
                                   ScheduleID const,
                                   HLTGlobalStatus&);

  private:
    void produce(Event&) override;

    fhicl::ParameterSetID pset_id_;
    cet::exempt_ptr<HLTGlobalStatus> trptr_;
  };
} // namespace art

#endif /* art_Framework_Core_TriggerResultInserter_h */

// Local Variables:
// mode: c++
// End:
