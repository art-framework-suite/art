// ======================================================================
// Produces a PtrVector instance.
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "fhiclcpp/types/Atom.h"

#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class DerivedPtrVectorProducer;
}

using arttest::DerivedPtrVectorProducer;

// ----------------------------------------------------------------------

class arttest::DerivedPtrVectorProducer : public art::EDProducer {
public:
  using input_t = std::vector<arttest::SimpleDerived>;
  using derived_t = art::PtrVector<arttest::SimpleDerived>;
  using base_t = art::PtrVector<arttest::Simple>;

  struct Config {
    fhicl::Atom<std::string> input_label{fhicl::Name{"input_label"}};
  };
  using Parameters = Table<Config>;
  explicit DerivedPtrVectorProducer(Parameters const& p)
    : inputToken_{
        consumes<input_t>(art::InputTag{p().input_label(), "derived"})}
  {
    produces<derived_t>();
    produces<base_t>();
  }

private:
  void produce(art::Event& e) override;

  art::ProductToken<input_t> const inputToken_;
}; // DerivedPtrVectorProducer

// ----------------------------------------------------------------------

void
DerivedPtrVectorProducer::produce(art::Event& e)
{
  auto const& h = e.getValidHandle(inputToken_);

  auto prod = std::make_unique<derived_t>();
  for (std::size_t k = 0; k != 16; ++k) {
    art::Ptr<SimpleDerived> p{h, k};
    prod->push_back(p);
  }
  auto base_prod = std::make_unique<base_t>(*prod);
  e.put(move(prod));
  e.put(move(base_prod));
}

DEFINE_ART_MODULE(DerivedPtrVectorProducer)
