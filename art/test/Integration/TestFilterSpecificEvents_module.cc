#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Sequence.h"

#include <vector>

namespace {
  using namespace fhicl;
  struct Config {
    Sequence<unsigned> eventsToAccept{Name("eventsToAccept")};
  };
} // namespace

namespace art {
  namespace test {
    class TestFilterSpecificEvents;
  }
} // namespace art

class art::test::TestFilterSpecificEvents : public EDFilter {
public:
  using Parameters = EDFilter::Table<Config>;

  explicit TestFilterSpecificEvents(Parameters const& ps)
    : eventsToAccept_{ps().eventsToAccept()}
  {
    cet::sort_all(eventsToAccept_);
  }

  bool
  filter(art::Event& e) override
  {
    return cet::binary_search_all(eventsToAccept_, e.event());
  }

private:
  std::vector<unsigned> eventsToAccept_;
};

DEFINE_ART_MODULE(art::test::TestFilterSpecificEvents)
