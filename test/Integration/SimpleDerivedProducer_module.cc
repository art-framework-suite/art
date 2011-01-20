
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <memory>
#include <vector>

namespace arttest {
  class SimpleDerivedProducer;
}

using arttest::SimpleDerivedProducer;

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class arttest::SimpleDerivedProducer
  : public art::EDProducer
{
  typedef  std::vector<arttest::SimpleDerived>  SimpleDerivedProduct;

public:
  explicit SimpleDerivedProducer( fhicl::ParameterSet const & p )
  : size_( p.get<int>("size") )
  {
    produces<SimpleDerivedProduct>("derived");
  }

  virtual ~SimpleDerivedProducer() { }
  virtual void produce( art::Event & e );

private:
  int size_;  // number of Simples to put in the collection
};

void
  SimpleDerivedProducer::produce( art::Event & e )
{
  // Fill up a collection of SimpleDerived objects
  std::auto_ptr<SimpleDerivedProduct> prod(new SimpleDerivedProduct);
  for (int i = 0; i != size_; ++i) {
    SimpleDerived sd;
    sd.key = size_ - i;
    sd.value = 1.5 * i + 100.0;
    sd.dummy = 0.0;
    prod->push_back(sd);
  }

  // Put the product into the Event
  e.put(prod, "derived");
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(SimpleDerivedProducer);

// ======================================================================
