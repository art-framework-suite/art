//--------------------------------------------------------------------
//
// Empty module just to test timeTracker stuff
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {

  class TestTimeTrackerProducer : public art::ReplicatedProducer {
  public:
    explicit TestTimeTrackerProducer(fhicl::ParameterSet const&,
                                     art::ScheduleID)
    {}

    void produce(art::Event&) override
    {}
  };

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestTimeTrackerProducer)
