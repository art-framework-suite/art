//--------------------------------------------------------------------
//
// Empty module just to test timeTracker stuff
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {

  class TestTimeTrackerAnalyzer : public art::EDAnalyzer
  {
  public:
    explicit TestTimeTrackerAnalyzer( fhicl::ParameterSet const& pset ) : art::EDAnalyzer(pset) {}
    virtual ~TestTimeTrackerAnalyzer() { }

    virtual void analyze( art::Event const& ) override {}
  };  // TestTimeTrackerAnalyzer

}

DEFINE_ART_MODULE(arttest::TestTimeTrackerAnalyzer)
