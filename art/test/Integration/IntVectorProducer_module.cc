// ======================================================================
//
// Produces an IntProduct instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class IntVectorProducer;
}

using arttest::IntVectorProducer;

// ----------------------------------------------------------------------

class arttest::IntVectorProducer : public art::EDProducer {
  using intvector_t = std::vector<int>;

public:
  struct Config {
    fhicl::Atom<unsigned> nvalues{fhicl::Name{"nvalues"}};
  };
  using Parameters = Table<Config>;
  explicit IntVectorProducer(Parameters const& p)
    : EDProducer{p}, nvalues_{p().nvalues()}
  {
    produces<intvector_t>();
  }

private:
  void
  produce(art::Event& e) override
  {
    auto const value = e.id().event();
    auto p = std::make_unique<intvector_t>();
    for (unsigned k = 0; k != nvalues_; ++k)
      p->push_back(value + k);
    e.put(move(p));
  }

  unsigned const nvalues_;

}; // IntVectorProducer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(IntVectorProducer)

// ======================================================================
