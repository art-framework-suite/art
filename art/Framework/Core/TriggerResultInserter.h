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
#include "art/Framework/Core/FCPfwd.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

// ----------------------------------------------------------------------

namespace art {
  class Event;
  class HLTGlobalStatus;

  class TriggerResultInserter
    : public art::EDProducer
  {
  public:
    typedef std::shared_ptr<HLTGlobalStatus> TrigResPtr;

    // standard constructor not supported for this module
    explicit TriggerResultInserter(fhicl::ParameterSet const& ps);

    // the pset needed here is the one that defines the trigger path names
    TriggerResultInserter(fhicl::ParameterSet const& ps, const TrigResPtr& trptr);
    virtual ~TriggerResultInserter();

    virtual void produce(art::Event& e);

  private:
    TrigResPtr trptr_;
    fhicl::ParameterSetID pset_id_;
  };  // TriggerResultInserter

}  // art

// ======================================================================

#endif /* art_Framework_Core_TriggerResultInserter_h */

// Local Variables:
// mode: c++
// End:
