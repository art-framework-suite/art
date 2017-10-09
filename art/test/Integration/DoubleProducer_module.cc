//--------------------------------------------------------------------
//
// Produces an DoubleProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <memory>

namespace arttest {
  class DoubleProducer;
}

class arttest::DoubleProducer : public art::EDProducer {
public:
  explicit DoubleProducer(fhicl::ParameterSet const& p)
    : value_(p.get<double>("dvalue"))
  {
    produces<DoubleProduct>();
  }

  explicit DoubleProducer(double d) : value_(d) { produces<DoubleProduct>(); }

  void produce(art::Event& e) override;

private:
  double value_;
}; // DoubleProducer

void
arttest::DoubleProducer::produce(art::Event& e)
{
  std::cerr << "Holy cow, DoubleProducer::produce is running!\n";
  std::unique_ptr<DoubleProduct> p(new DoubleProduct(value_));
  e.put(std::move(p));
}

DEFINE_ART_MODULE(arttest::DoubleProducer)
