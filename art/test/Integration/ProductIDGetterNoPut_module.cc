////////////////////////////////////////////////////////////////////////
// Class:       ProductIDGetterNoPut
// Module Type: producer
// File:        ProductIDGetterNoPut_module.cc
////////////////////////////////////////////////////////////////////////

#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/ProductID.h"

#include <vector>

namespace art {
  namespace test {
    class ProductIDGetterNoPut;
  }
}

class art::test::ProductIDGetterNoPut : public EDProducer {
public:
  explicit ProductIDGetterNoPut(fhicl::ParameterSet const&);
  void produce(art::Event&) override;
};

art::test::ProductIDGetterNoPut::ProductIDGetterNoPut(
  fhicl::ParameterSet const&)
{
  produces<int>();
  produces<int>("i1");
}

void
art::test::ProductIDGetterNoPut::produce(Event&)
{
  ProductID const p1{getProductID<int>()};
  BOOST_REQUIRE(p1.isValid());
  ProductID const p2{getProductID<int>("i1")};
  BOOST_REQUIRE(p2.isValid());
  BOOST_REQUIRE_NE(p1, p2);
}

DEFINE_ART_MODULE(art::test::ProductIDGetterNoPut)
