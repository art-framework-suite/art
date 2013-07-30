////////////////////////////////////////////////////////////////////////
// Class:       ProductIDGetter
// Module Type: producer
// File:        ProductIDGetter_module.cc
//
// Generated at Wed Jun 15 17:19:52 2011 by Chris Green using artmod
// from art v0_07_09.
////////////////////////////////////////////////////////////////////////

#include "boost/test/included/unit_test.hpp"

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Provenance/ProductID.h"

#include <vector>

namespace arttest {
  class ProductIDGetter;
}

class arttest::ProductIDGetter : public art::EDProducer {
public:
  explicit ProductIDGetter(fhicl::ParameterSet const &p);
  virtual ~ProductIDGetter();

  virtual void produce(art::Event &e);

private:

};


arttest::ProductIDGetter::ProductIDGetter(fhicl::ParameterSet const &)
{
  produces<int>();
  produces<int>("i1");
  produces<std::vector<int> >();
  produces<art::Ptr<int> >();
}

arttest::ProductIDGetter::~ProductIDGetter() {
}

void arttest::ProductIDGetter::produce(art::Event &e) {
  art::ProductID p1(getProductID<int>(e));
  BOOST_REQUIRE(p1.isValid());
  art::ProductID p2(getProductID<int>(e, "i1"));
  BOOST_REQUIRE(p2.isValid());
  BOOST_REQUIRE_NE(p1, p2);
  std::unique_ptr<std::vector<int> > vip(new std::vector<int>);
  vip->push_back(0);
  vip->push_back(2);
  vip->push_back(4);
  vip->push_back(6);

  art::ProductID pv(getProductID<std::vector<int> >(e));
  std::unique_ptr<art::Ptr<int> >ptr(new art::Ptr<int>(pv, 2, e.productGetter(pv)));

  BOOST_REQUIRE(ptr->id().isValid());

  art::ProductID id(e.put(std::move(vip)));

  art::Ptr<int> ptr_check(id, 2, e.productGetter(id));

  BOOST_REQUIRE_EQUAL(ptr->id(), ptr_check.id());

  e.put(std::move(ptr));
}

DEFINE_ART_MODULE(arttest::ProductIDGetter)
