//--------------------------------------------------------------------
// Empty module just to test TimeTracker stuff
//--------------------------------------------------------------------

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedFilter.h"
#include "art/Framework/Principal/Event.h"

namespace arttest {

  class TestTimeTrackerFilter : public art::SharedFilter {
  public:
    struct Config {
    };
    using Parameters = Table<Config>;
    explicit TestTimeTrackerFilter(Parameters const& p) : art::SharedFilter{p}
    {}

  private:
    bool
    filter(art::Event& e, art::ScheduleID) override
    {
      bool const passesCuts = (e.event() % 10) < 3;
      return passesCuts;
    }
  }; // TestTimeTrackerFilter

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestTimeTrackerFilter)
