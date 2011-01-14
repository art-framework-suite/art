// ======================================================================
//
// Produces an IntProduct instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "fhiclcpp/ParameterSet.h"
//#include "test/TestObjects/ToyProducts.h"
#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class IntVectorProducer;
}

using arttest::IntVectorProducer;

// ----------------------------------------------------------------------

class arttest::IntVectorProducer
  : public art::EDProducer
{
public:
  typedef  std::vector<int>  intvector_t;

  explicit IntVectorProducer( fhicl::ParameterSet const & p )
  : value_  ( p.get<int     >("ivalue") )
  , nvalues_( p.get<unsigned>("nvalues") )
  {
    produces<intvector_t>();
  }

  explicit IntVectorProducer( int i, unsigned n )
  : value_  ( i )
  , nvalues_( n )
  {
    produces<intvector_t>();
  }

  virtual ~IntVectorProducer() { }

  virtual void produce( art::Event& e );

private:
  int      value_;
  unsigned nvalues_;

};  // IntVectorProducer

// ----------------------------------------------------------------------

void
IntVectorProducer::produce( art::Event& e )
{
  std::cerr << "IntVectorProducer::produce is running!\n";

  std::auto_ptr<intvector_t> p( new intvector_t(nvalues_, value_) );
  e.put(p);
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(IntVectorProducer);

// ======================================================================
