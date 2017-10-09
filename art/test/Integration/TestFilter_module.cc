#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"

#include "fhiclcpp/types/Atom.h"

#include <cassert>

namespace {
  using namespace fhicl;
  struct Config {
    Atom<int> acceptValue{Name("acceptValue"), 1};
    Atom<bool> onlyOne{Name("onlyOne"), false};
  };
}

namespace arttest {
  class TestFilter;
}

class arttest::TestFilter : public art::EDFilter {
public:
  using Parameters = EDFilter::Table<Config>;
  explicit TestFilter(EDFilter::Table<Config> const&);

  bool filter(art::Event& e) override;
  void endJob() override;

private:
  int count_;
  int accept_rate_; // how many out of 100 will be accepted?
  bool onlyOne_;
};

// -------

// -----------------------------------------------------------------

arttest::TestFilter::TestFilter(EDFilter::Table<Config> const& ps)
  : count_(), accept_rate_(ps().acceptValue()), onlyOne_(ps().onlyOne())
{}

bool
arttest::TestFilter::filter(art::Event&)
{
  ++count_;
  assert(currentContext() != 0);
  if (onlyOne_)
    return count_ % accept_rate_ == 0;
  else
    return count_ % 100 <= accept_rate_;
}

void
arttest::TestFilter::endJob()
{
  assert(currentContext() == 0);
}

DEFINE_ART_MODULE(arttest::TestFilter)
