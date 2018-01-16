//--------------------------------------------------------------------
//
// Empty module just to test TimeTracker stuff
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"

namespace arttest {

  class TestTimeTrackerFilter : public art::EDFilter {
  public:
    struct Config {
    };
    using Parameters = art::EDFilter::Table<Config>;
    explicit TestTimeTrackerFilter(Parameters const&) {}

  private:
    bool
    filter(art::Event& e) override
    {
      bool const passesCuts = (e.event() % 10) < 3;
      return passesCuts;
    }
  }; // TestTimeTrackerFilter
}

DEFINE_ART_MODULE(arttest::TestTimeTrackerFilter)
