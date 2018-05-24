#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"

#include <algorithm>
#include <cassert>
#include <vector>

using namespace fhicl;

namespace {
  class IntsVerifier : public art::SharedProducer {
  public:
    struct Config {
      Sequence<int> expectedValues{
        Name{"expectedValues"},
        Comment{
          "The 'expectedValues' parameter lists the values that\n"
          "are intended to be present whenever a getManyByType call is made."}};
    };
    using Parameters = Table<Config>;
    explicit IntsVerifier(Parameters const&);

  private:
    void produce(art::Event&, art::Services const&) override;
    std::vector<int> expectedValues_;
  };

  IntsVerifier::IntsVerifier(Parameters const& p)
    : art::SharedProducer{p}, expectedValues_{p().expectedValues()}
  {
    std::sort(begin(expectedValues_), end(expectedValues_));
    async<art::InEvent>();
  }

  void
  IntsVerifier::produce(art::Event& e, art::Services const&)
  {
    std::vector<art::Handle<arttest::IntProduct>> products;
    e.getManyByType(products);
    std::set<int> presentValues;
    for (auto const& h : products) {
      presentValues.insert(h->value);
    }
    assert(presentValues.size() == expectedValues_.size());
    auto const sameValues = std::equal(
      cbegin(presentValues), cend(presentValues), cbegin(expectedValues_));
    assert(sameValues);
  }
}

DEFINE_ART_MODULE(IntsVerifier)
