////////////////////////////////////////////////////////////////////////
// Class:       ProductIDGetter
// Module Type: producer
// File:        ProductIDGetter_module.cc
//
// Generated at Wed Jun 15 17:19:52 2011 by Chris Green using artmod
// from art v0_07_09.
////////////////////////////////////////////////////////////////////////

#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/ProductID.h"

#include <vector>

namespace arttest {
  class ProductIDGetter;
}

class arttest::ProductIDGetter : public art::EDProducer {
public:

  explicit ProductIDGetter(fhicl::ParameterSet const &);
  void produce(art::Event &) override;

};


arttest::ProductIDGetter::ProductIDGetter(fhicl::ParameterSet const &)
{
  produces<std::vector<int> >();
  produces<art::Ptr<int> >();
}

void arttest::ProductIDGetter::produce(art::Event &e) {
  auto vip = std::make_unique<std::vector<int>>();
  vip->push_back(0);
  vip->push_back(2);
  vip->push_back(4);
  vip->push_back(6);

  art::ProductID pv(getProductID<std::vector<int> >(e));
  auto ptr = std::make_unique<art::Ptr<int>>(pv, 2, e.productGetter(pv));

  BOOST_REQUIRE(ptr->id().isValid());

  art::ProductID id(e.put(std::move(vip)));

  art::Ptr<int> ptr_check(id, 2, e.productGetter(id));

  BOOST_REQUIRE_EQUAL(ptr->id(), ptr_check.id());

  BOOST_REQUIRE(!ptr_check.isAvailable());

  e.put(std::move(ptr));
}

DEFINE_ART_MODULE(arttest::ProductIDGetter)
