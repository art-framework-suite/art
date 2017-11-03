// ======================================================================
// Verifies the values of an IntArray instance.
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>

namespace {
  constexpr std::size_t sz{4u};
}

namespace arttest {
  class IntArrayAnalyzer;
}

class arttest::IntArrayAnalyzer : public art::EDAnalyzer {
  art::ProductToken<IntArray<sz>> arrayToken_;
public:

  struct Config{
    fhicl::Atom<std::string> moduleLabel{fhicl::Name{"moduleLabel"}};
  };
  using Parameters = Table<Config>;

  explicit IntArrayAnalyzer(Parameters const& p) :
    art::EDAnalyzer{p},
    arrayToken_{consumes<IntArray<sz>>(p().moduleLabel())}
  {}

  void
  analyze(art::Event const& e) override
  {
    // Produce reference
    int const value = e.id().event();
    std::array<int, sz> ref{{}};
    auto p = std::make_unique<IntArray<sz>>();
    for (int k = 0; k != sz; ++k) {
      ref[k] = value+k;
    }

    auto const& prod = *e.getValidHandle(arrayToken_);
    for (int k = 0; k != sz; ++k) {
      assert(ref[k] == prod.arr[k]);
    }
  }

}; // IntArrayAnalyzer

DEFINE_ART_MODULE(arttest::IntArrayAnalyzer)
