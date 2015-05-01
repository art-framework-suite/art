////////////////////////////////////////////////////////////////////////
// Class:       AssnsProducer
// Module Type: producer
// File:        AssnsProducer_module.cc
//
// Generated at Thu Jul  7 13:34:45 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/AssnTestData.h"

#include "cetlib/map_vector.h"

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::make_unique;

using art::Ptr;
using art::ProductID;

using uintvec = vector<size_t>;
using stringvec = vector<string>;
using mapvec = cet::map_vector<string>;

namespace arttest {
  class AssnsProducer;
}

class arttest::AssnsProducer : public art::EDProducer {
public:
  explicit AssnsProducer(fhicl::ParameterSet const &p);
  virtual ~AssnsProducer();

  virtual void produce(art::Event &e);
};

namespace {
  typedef art::Assns<size_t, string, arttest::AssnTestData> AssnsAB_t;
  typedef art::Assns<string, size_t, arttest::AssnTestData> AssnsBA_t;
  typedef art::Assns<size_t, string> AssnsVoid_t;
}

arttest::AssnsProducer::AssnsProducer(fhicl::ParameterSet const &)
{
  produces<uintvec>();
  produces<stringvec>();
  produces<mapvec>("mv");
  produces<AssnsAB_t>();
  produces<AssnsAB_t>("mapvec");
  produces<AssnsVoid_t>();
  produces<AssnsVoid_t>("mapvec");
  produces<AssnsAB_t>("many");
  produces<AssnsAB_t>("manymapvec");
  produces<AssnsVoid_t>("many");
  produces<AssnsVoid_t>("manymapvec");
}

arttest::AssnsProducer::~AssnsProducer() {
}

void arttest::AssnsProducer::produce(art::Event &e) {

  // Create the data products among which we will make associations.
  auto vui = make_unique<uintvec>(uintvec { 2, 0, 1 } );
  auto vs = make_unique<stringvec>(stringvec {"one", "two", "zero"});

  // Making a map_vector is hard.
  auto mvs = make_unique<mapvec>();
  using key_t = mapvec::key_type;
  mvs->reserve(3);
  (*mvs)[key_t(0)] = "zero";
  (*mvs)[key_t(11)] = "one";
  (*mvs)[key_t(22)] = "two";

  // We will need the product IDs of the data products.
  ProductID vui_pid = getProductID<uintvec>(e);
  ProductID vs_pid = getProductID<stringvec>(e);
  ProductID mvs_pid = getProductID<mapvec>(e, "mv");

  // Create the association objects.
  // Assns into vectors.
  std::unique_ptr<AssnsAB_t>    a(new AssnsAB_t);
  std::unique_ptr<AssnsVoid_t> av(new AssnsVoid_t);
  // Assns into map_vector.
  std::unique_ptr<AssnsAB_t>    b(new AssnsAB_t);
  std::unique_ptr<AssnsVoid_t> bv(new AssnsVoid_t);

  // addS will add to both x and xv a referenece between slot1 of
  // productID1 and slot2 of productID2. The reference in x will have
  // associated data td.
  auto addS = [&e](auto& x,
       auto& xv,
       ProductID id1, int slot1,
       ProductID id2, int slot2,
       auto td)
    {
      x->addSingle(Ptr<size_t>(id1, slot1, e.productGetter(id1)),
       Ptr<string>(id2, slot2, e.productGetter(id2)),
       td);
      xv->addSingle(Ptr<size_t>(id1, slot1, e.productGetter(id1)),
        Ptr<string>(id2, slot2, e.productGetter(id2)));
    };

  // We add associations in an order such that the associated data are
  // in alphabetical order.
  addS(a, av, vui_pid, 1, vs_pid, 2, AssnTestData(1,2,"A"));
  addS(b, bv, vui_pid, 1, mvs_pid, 0, AssnTestData(1,0,"A"));

  addS(a, av, vui_pid, 2, vs_pid, 0, AssnTestData(2,0,"B"));
  addS(b, bv, vui_pid, 2, mvs_pid, 11, AssnTestData(2,11,"B"));

  addS(a, av, vui_pid, 0, vs_pid, 1, AssnTestData(0,1,"C"));
  addS(b, bv, vui_pid, 0, mvs_pid, 22, AssnTestData(0,22,"C"));

  auto am = make_unique<AssnsAB_t>(*a);
  auto avm = make_unique<AssnsVoid_t>(*av);
  auto bm = make_unique<AssnsAB_t>(*b);
  auto bvm = make_unique<AssnsVoid_t>(*bv);

  addS(am, avm, vui_pid, 1, vs_pid, 2, AssnTestData(1,2,"AA"));
  addS(bm, bvm, vui_pid, 1, mvs_pid, 0, AssnTestData(1,0,"AA"));

  e.put(std::move(vui));
  e.put(std::move(vs));
  e.put(std::move(mvs), "mv");
  e.put(std::move(a));
  e.put(std::move(av));
  e.put(std::move(am), "many");
  e.put(std::move(avm), "many");
  e.put(std::move(b), "mapvec");
  e.put(std::move(bv), "mapvec");
  e.put(std::move(bm), "manymapvec");
  e.put(std::move(bvm), "manymapvec");
}

DEFINE_ART_MODULE(arttest::AssnsProducer)
