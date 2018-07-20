#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"

#include "fhiclcpp/types/Atom.h"

#include <cassert>

namespace {
  using namespace fhicl;
  struct Config {
    Atom<unsigned> acceptValue{Name("acceptValue"), 1};
    Atom<bool> onlyOne{Name("onlyOne"), false};
  };
} // namespace

namespace arttest {
  class TestFilter;
}

class arttest::TestFilter : public art::EDFilter {
public:
  using Parameters = Table<Config>;
  explicit TestFilter(Parameters const&);

private:
  bool filter(art::Event& e) override;

  unsigned count_{};
  unsigned const acceptRate_; // how many out of 100 will be accepted?
  bool const onlyOne_;
};

// -------

// -----------------------------------------------------------------

arttest::TestFilter::TestFilter(Parameters const& ps)
  : EDFilter{ps}, acceptRate_{ps().acceptValue()}, onlyOne_{ps().onlyOne()}
{}

bool
arttest::TestFilter::filter(art::Event&)
{
  ++count_;
  if (onlyOne_)
    return count_ % acceptRate_ == 0;
  else
    return count_ % 100 <= acceptRate_;
}

DEFINE_ART_MODULE(arttest::TestFilter)
