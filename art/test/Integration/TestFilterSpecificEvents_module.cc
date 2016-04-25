#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Sequence.h"

#include <vector>

namespace {
  using namespace fhicl;
  struct Config {
    Sequence<unsigned> eventsToAccept { Name("eventsToAccept") };
  };
}

namespace arttest {
  class TestFilterSpecificEvents;
}

class arttest::TestFilterSpecificEvents : public art::EDFilter {
public:

  using Parameters = EDFilter::Table<Config>;
  explicit TestFilterSpecificEvents(EDFilter::Table<Config> const&);

  bool filter(art::Event& e) override;

private:
  std::vector<unsigned> eventsToAccept_;
};

// -------

// -----------------------------------------------------------------

arttest::TestFilterSpecificEvents::TestFilterSpecificEvents(EDFilter::Table<Config> const& ps)
  : eventsToAccept_{ps().eventsToAccept()}
{
  cet::sort_all(eventsToAccept_);
}

bool arttest::TestFilterSpecificEvents::filter(art::Event& e)
{
  return cet::binary_search_all(eventsToAccept_, e.event());
}

DEFINE_ART_MODULE(arttest::TestFilterSpecificEvents)
