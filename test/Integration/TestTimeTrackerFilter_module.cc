//--------------------------------------------------------------------
//
// Empty module just to test timeTracker stuff
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <random>

namespace arttest {

  class TestTimeTrackerFilter : public art::EDFilter
  {
  public:
    explicit TestTimeTrackerFilter( fhicl::ParameterSet const& ) {}
    virtual ~TestTimeTrackerFilter() { }

    virtual bool filter( art::Event& ) override {
      bool const passesCuts = rand_(dre_) < 0.3;

      return
        passesCuts ?
        EDFilter::Pass :
        EDFilter::Fail ;
    }
  private:
    std::default_random_engine dre_;
    std::uniform_real_distribution<> rand_;
  };  // TestTimeTrackerFilter

}

DEFINE_ART_MODULE(arttest::TestTimeTrackerFilter)
