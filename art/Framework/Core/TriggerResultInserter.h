#ifndef art_Framework_Core_TriggerResultInserter_h
#define art_Framework_Core_TriggerResultInserter_h

// ======================================================================
//
// TriggerResultInserter - This is an unusual module in that it is always
// present in the schedule and it is not configurable.  The ownership of
// the bitmask is shared with the scheduler.  Its purpose is to create a
// TriggerResults instance and insert it into the event.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

// ----------------------------------------------------------------------

namespace art {
  class Event;
  class HLTGlobalStatus;

  class TriggerResultInserter : public art::EDProducer {
  public:
    typedef cet::exempt_ptr<HLTGlobalStatus> TrigResPtr;

    // standard constructor not supported for this module
    explicit TriggerResultInserter(fhicl::ParameterSet const& ps) = delete;

    // the pset needed here is the one that defines the trigger path names
    TriggerResultInserter(fhicl::ParameterSet const& ps,
                          HLTGlobalStatus& pathResults);

    void produce(art::Event& e) override;

  private:
    TrigResPtr trptr_;
    fhicl::ParameterSetID pset_id_;
  }; // TriggerResultInserter

} // art

  // ======================================================================

#endif /* art_Framework_Core_TriggerResultInserter_h */

// Local Variables:
// mode: c++
// End:
