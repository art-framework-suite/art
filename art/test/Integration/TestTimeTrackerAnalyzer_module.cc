//--------------------------------------------------------------------
//
// Empty module just to test timeTracker stuff
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <thread>

namespace arttest {

  class TestTimeTrackerAnalyzer : public art::EDAnalyzer {
  public:
    explicit TestTimeTrackerAnalyzer(fhicl::ParameterSet const& pset)
      : art::EDAnalyzer(pset)
    {}

    void
    analyze(art::Event const&) override
    {
      using namespace std::literals;
      std::this_thread::sleep_for(10ms * scheduleID().id());
    }
  }; // TestTimeTrackerAnalyzer

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestTimeTrackerAnalyzer)
