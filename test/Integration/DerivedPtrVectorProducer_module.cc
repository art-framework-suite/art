// ======================================================================
//
// Produces a PtrVector instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <iostream>
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
  typedef  art::PtrVector<arttest::SimpleDerived>  derived_t;
  typedef  art::PtrVector<arttest::Simple>  base_t;

  explicit DerivedPtrVectorProducer( fhicl::ParameterSet const & p )
  : input_label_( p.get<std::string>("input_label") )
  {
    produces<derived_t>();
    produces<base_t>();
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

  std::unique_ptr<derived_t> prod( new derived_t );
  for( int k = 0; k != 16; ++k ) {
    art::Ptr<SimpleDerived> p(h, k);
    prod->push_back(p);
  }
  std::unique_ptr<base_t> base_prod(new base_t(*prod));
  e.put(std::move(prod));
  e.put(std::move(base_prod));
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(DerivedPtrVectorProducer)

// ======================================================================
