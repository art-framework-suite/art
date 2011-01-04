//--------------------------------------------------------------------
//
// Produces an IntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Common/Handle.h"
#include "test/TestObjects/ToyProducts.h"
#include <memory>
#include <iostream>

namespace arttest {
  class IntProducer;
}

using arttest::IntProducer;

class arttest::IntProducer
  : public art::EDProducer
{
public:
  explicit IntProducer( fhicl::ParameterSet const& p )
  : value_( p.get<int>("ivalue") )
  {
    produces<IntProduct>();
  }

  explicit IntProducer( int i )
  : value_(i)
  {
    produces<IntProduct>();
  }

  virtual ~IntProducer() { }

  virtual void produce( art::Event& e );

private:
  int value_;
};  // IntProducer

void
IntProducer::produce( art::Event& e )
{
  std::cerr << "Holy cow, IntProducer::produce is running!\n";
  std::auto_ptr<IntProduct> p(new IntProduct(value_));
  e.put(p);
}

DEFINE_ART_MODULE(IntProducer);
