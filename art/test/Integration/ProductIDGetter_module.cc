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

namespace art {
  namespace test {
    class ProductIDGetter;
  }
} // namespace art

class art::test::ProductIDGetter : public EDProducer {
public:
  struct Config {
  };
  using Parameters = Table<Config>;
  explicit ProductIDGetter(Parameters const&);

private:
  void beginSubRun(art::SubRun&) override;
  void produce(art::Event&) override;
};

art::test::ProductIDGetter::ProductIDGetter(Parameters const& ps)
  : EDProducer{ps}
{
  produces<std::vector<int>, art::InSubRun>();
  produces<art::Ptr<int>, art::InSubRun>();
  produces<std::vector<int>>();
  produces<art::Ptr<int>>();
}

void
art::test::ProductIDGetter::beginSubRun(art::SubRun& sr)
{
  auto vip = std::make_unique<std::vector<int>>();
  vip->push_back(1);
  vip->push_back(3);
  vip->push_back(5);
  vip->push_back(7);

  art::ProductID const pv{sr.getProductID<std::vector<int>>()};
  auto ptr = std::make_unique<art::Ptr<int>>(pv, 2, sr.productGetter(pv));

  BOOST_REQUIRE(ptr->id().isValid());

  art::ProductID const id{sr.put(std::move(vip))};
  art::Ptr<int> const ptr_check{id, 2, sr.productGetter(id)};

  BOOST_REQUIRE_EQUAL(ptr->id(), ptr_check.id());
  BOOST_REQUIRE(!ptr_check.isAvailable());

  sr.put(std::move(ptr), art::fullSubRun());
}

void
art::test::ProductIDGetter::produce(art::Event& e)
{
  // Test that getting a ProductID for an unregistered product yields an
  // exception.
  BOOST_REQUIRE_EXCEPTION(
    e.getProductID<int>(), art::Exception, [](art::Exception const& e) {
      return e.categoryCode() == art::errors::ProductRegistrationFailure;
    });

  auto vip = std::make_unique<std::vector<int>>();
  vip->push_back(0);
  vip->push_back(2);
  vip->push_back(4);
  vip->push_back(6);

  art::ProductID const pv{e.getProductID<std::vector<int>>()};
  auto ptr = std::make_unique<art::Ptr<int>>(pv, 2, e.productGetter(pv));

  BOOST_REQUIRE(ptr->id().isValid());

  art::ProductID const id{e.put(std::move(vip))};
  art::Ptr<int> const ptr_check{id, 2, e.productGetter(id)};

  BOOST_REQUIRE_EQUAL(ptr->id(), ptr_check.id());
  BOOST_REQUIRE(!ptr_check.isAvailable());

  e.put(std::move(ptr));
}

DEFINE_ART_MODULE(art::test::ProductIDGetter)
