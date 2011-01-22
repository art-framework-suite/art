// ======================================================================
//
// Produces a PtrVector instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class DerivedPtrVectorProducer;
}

using arttest::DerivedPtrVectorProducer;

// ----------------------------------------------------------------------

class arttest::DerivedPtrVectorProducer
  : public art::EDProducer
{
public:
  typedef  std::vector<arttest::SimpleDerived>     input_t;
  typedef  art::PtrVector<arttest::SimpleDerived>  product_t;

  explicit DerivedPtrVectorProducer( fhicl::ParameterSet const & p )
  : input_label_( p.get<std::string>("input_label") )
  {
    produces<product_t>();
  }

  virtual ~DerivedPtrVectorProducer() { }

  virtual void produce( art::Event & e );

private:
  std::string  input_label_;

};  // DerivedPtrVectorProducer

// ----------------------------------------------------------------------

void
DerivedPtrVectorProducer::produce( art::Event& e )
{
  std::cerr << "DerivedPtrVectorProducer::produce is running!\n";

  art::Handle<input_t> h;
  e.getByLabel(input_label_, "derived", h);

  std::auto_ptr<product_t> prod( new product_t );
  for( int k = 0; k != 16; ++k ) {
    art::Ptr<SimpleDerived> p(h, k, true);
    prod->push_back(p);
  }

  e.put(prod);
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(DerivedPtrVectorProducer);

// ======================================================================
