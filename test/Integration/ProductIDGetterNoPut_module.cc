////////////////////////////////////////////////////////////////////////
// Class:       ProductIDGetterNoPut
// Module Type: producer
// File:        ProductIDGetterNoPut_module.cc
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/quiet_unit_test.hpp"

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Provenance/ProductID.h"

#include <vector>

namespace arttest {
  class ProductIDGetterNoPut;
}

class arttest::ProductIDGetterNoPut : public art::EDProducer {
public:
  
  explicit ProductIDGetterNoPut(fhicl::ParameterSet const &);
  virtual void produce(art::Event &);

};


arttest::ProductIDGetterNoPut::ProductIDGetterNoPut(fhicl::ParameterSet const &)
{
  produces<int>();
  produces<int>("i1");
}

void arttest::ProductIDGetterNoPut::produce(art::Event &e) {
  art::ProductID p1(getProductID<int>(e));
  BOOST_REQUIRE(p1.isValid());
  art::ProductID p2(getProductID<int>(e, "i1"));
  BOOST_REQUIRE(p2.isValid());
  BOOST_REQUIRE_NE(p1, p2);
}

DEFINE_ART_MODULE(arttest::ProductIDGetterNoPut)
