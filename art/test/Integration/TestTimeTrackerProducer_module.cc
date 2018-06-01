//--------------------------------------------------------------------
// Empty module just to test timeTracker stuff
//--------------------------------------------------------------------

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/ReplicatedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {

  class TestTimeTrackerProducer : public art::ReplicatedProducer {
  public:
    struct Config {
    };
    using Parameters = Table<Config>;
    explicit TestTimeTrackerProducer(Parameters const& p,
                                     art::ProcessingFrame const& frame)
      : art::ReplicatedProducer{p, frame}
    {}

    void
    produce(art::Event&, art::ProcessingFrame const&) override
    {}
  };

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestTimeTrackerProducer)
