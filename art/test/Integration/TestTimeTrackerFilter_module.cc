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
    explicit TestTimeTrackerFilter(Parameters const& p,
                                   art::ProcessingFrame const&);

  private:
    bool filter(art::Event& e, art::ProcessingFrame const&) override;
  };

  TestTimeTrackerFilter::TestTimeTrackerFilter(Parameters const& p,
                                               art::ProcessingFrame const&)
    : art::SharedFilter{p}
  {
    async<art::InEvent>();
  }

  bool
  TestTimeTrackerFilter::filter(art::Event& e, art::ProcessingFrame const&)
  {
    bool const passesCuts = (e.event() % 10) < 3;
    return passesCuts;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestTimeTrackerFilter)
