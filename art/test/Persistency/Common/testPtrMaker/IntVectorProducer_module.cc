// ======================================================================
//
// Produces an IntProduct instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"

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
  : nvalues_( p.get<int>("nvalues") )
  {
    produces<intvector_t>();
  }

  void produce( art::Event & e ) override
  {
    std::cerr << "IntVectorProducer::produce is running!\n";
    int value_ = e.id().event();
    std::unique_ptr<intvector_t> p(new intvector_t);
    for( int k = 0; k != nvalues_; ++k ) {
      p->push_back(value_ * k);
    }
    e.put(std::move(p));
  }

private:
  int nvalues_;

};  // IntVectorProducer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(IntVectorProducer)

// ======================================================================
