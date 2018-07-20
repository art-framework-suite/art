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
#include "art/test/TestObjects/AssnTestData.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"

#include "cetlib/map_vector.h"

#include <memory>
#include <string>
#include <vector>

using std::make_unique;
using std::string;
using std::vector;

using art::ProductID;
using art::Ptr;

using uintvec = vector<size_t>;
using stringvec = vector<string>;
using mapvec = cet::map_vector<string>;

namespace arttest {
  class AssnsProducer;
}

class arttest::AssnsProducer : public art::EDProducer {
public:
  explicit AssnsProducer(fhicl::ParameterSet const& p);

private:
  void produce(art::Event& e) override;

  std::string const wantVoid_; // "ALL," "NONE," or "SOME."
  bool const wantMV_;          // Produce mapvector and derived Assns.
  bool const wantMany_;        // Produce many-to-many associations.
  bool const dinkVoid_;        // Distinguish ABV from ABD with extra entry.
  bool const wantAmbiguous_;   // Add an extra ABD to cause ambiguity.
};

namespace {
  using AssnsABX_t = art::Assns<size_t, string, arttest::AssnTestData>;
  using AssnsABY_t = art::Assns<size_t, string, string>;
  using AssnsVoid_t = art::Assns<size_t, string>;
} // namespace

arttest::AssnsProducer::AssnsProducer(fhicl::ParameterSet const& ps)
  : EDProducer{ps}
  , wantVoid_{ps.get<std::string>("wantVoid", "ALL")}
  , wantMV_{ps.get<bool>("wantMV", true)}
  , wantMany_{ps.get<bool>("wantMany", true)}
  , dinkVoid_{ps.get<bool>("dinkVoid", false)}
  , wantAmbiguous_{ps.get<bool>("wantAmbiguous", false)}
{
  produces<uintvec>();
  produces<stringvec>();
  if (wantMV_) {
    produces<mapvec>("mv");
    produces<AssnsABX_t>("mapvec");
    if (wantMany_) {
      produces<AssnsABX_t>("manymapvec");
    }
    if (wantVoid_ != "NONE") {
      produces<AssnsVoid_t>("mapvec");
      if (wantMany_) {
        produces<AssnsVoid_t>("manymapvec");
      }
    }
  }
  produces<AssnsABX_t>();
  if (wantMany_) {
    produces<AssnsABX_t>("many");
  }
  if (wantVoid_ == "ALL") {
    produces<AssnsVoid_t>();
    if (wantMany_) {
      produces<AssnsVoid_t>("many");
    }
  }
  if (wantAmbiguous_) {
    produces<AssnsABY_t>();
  }
}

void
arttest::AssnsProducer::produce(art::Event& e)
{

  // Create the data products among which we will make associations.
  auto vui = make_unique<uintvec>(uintvec{2, 0, 1});
  auto vs = make_unique<stringvec>(stringvec{"one", "two", "zero"});

  // Making a map_vector is hard.
  auto mvs = make_unique<mapvec>();
  using key_t = mapvec::key_type;
  mvs->reserve(4);
  (*mvs)[key_t(0)] = "zero";
  (*mvs)[key_t(11)] = "one";
  (*mvs)[key_t(22)] = "two";

  // Extra members.
  if (dinkVoid_) {
    vui->emplace_back(4);
    vs->emplace_back("four");
    (*mvs)[key_t(33)] = "four";
  }

  // We will need the product IDs of the data products.
  ProductID const vui_pid{e.getProductID<uintvec>()};
  ProductID const vs_pid{e.getProductID<stringvec>()};

  // Create the association objects.
  // Assns into vectors.
  auto a = std::make_unique<AssnsABX_t>();
  auto ay = std::make_unique<AssnsABY_t>();
  auto av = std::make_unique<AssnsVoid_t>();

  // Assns into map_vector.
  auto b = std::make_unique<AssnsABX_t>();
  auto bv = std::make_unique<AssnsVoid_t>();

  // addS will add to both x and xv a reference between slot1 of
  // productID1 and slot2 of productID2. The reference in x will have
  // associated data td.
  auto addS = [&e](auto& x,
                   auto& xv,
                   ProductID const id1,
                   size_t const slot1,
                   ProductID const id2,
                   size_t const slot2,
                   auto td) {
    Ptr<size_t> const p1{id1, slot1, e.productGetter(id1)};
    Ptr<string> const p2{id2, slot2, e.productGetter(id2)};
    x->addSingle(p1, p2, td);
    xv->addSingle(p1, p2);
  };
  // AddSV adds only to x, and has no td.
  auto addSV = [&e](auto& x,
                    ProductID const id1,
                    size_t const slot1,
                    ProductID const id2,
                    size_t const slot2) {
    Ptr<size_t> const p1{id1, slot1, e.productGetter(id1)};
    Ptr<string> const p2{id2, slot2, e.productGetter(id2)};
    x->addSingle(p1, p2);
  };
  // AddSS adds only to x (with td).
  auto addSS = [&e](auto& x,
                    ProductID const id1,
                    size_t const slot1,
                    ProductID const id2,
                    size_t const slot2,
                    auto td) {
    Ptr<size_t> const p1{id1, slot1, e.productGetter(id1)};
    Ptr<string> const p2{id2, slot2, e.productGetter(id2)};
    x->addSingle(p1, p2, td);
  };

  // We add associations in an order such that the associated data are
  // in alphabetical order.
  addS(a, av, vui_pid, 1, vs_pid, 2, AssnTestData(1, 2, "A"));
  addSS(ay, vui_pid, 1, vs_pid, 2, "A");

  addS(a, av, vui_pid, 2, vs_pid, 0, AssnTestData(2, 0, "B"));
  addSS(ay, vui_pid, 2, vs_pid, 0, "B");

  addS(a, av, vui_pid, 0, vs_pid, 1, AssnTestData(0, 1, "C"));
  addSS(ay, vui_pid, 0, vs_pid, 1, "C");

  if (dinkVoid_) {
    addSV(av, vui_pid, 3, vs_pid, 3);
  }

  auto am = make_unique<AssnsABX_t>(*a);
  auto avm = make_unique<AssnsVoid_t>(*av);

  addS(am, avm, vui_pid, 1, vs_pid, 2, AssnTestData(1, 2, "AA"));

  if (dinkVoid_) {
    addSV(avm, vui_pid, 3, vs_pid, 3);
  }

  std::unique_ptr<AssnsABX_t> bm;
  std::unique_ptr<AssnsVoid_t> bvm;

  if (wantMV_) {
    ProductID const mvs_pid{e.getProductID<mapvec>("mv")};
    addS(b, bv, vui_pid, 1, mvs_pid, 0, AssnTestData(1, 0, "A"));
    addS(b, bv, vui_pid, 2, mvs_pid, 11, AssnTestData(2, 11, "B"));
    addS(b, bv, vui_pid, 0, mvs_pid, 22, AssnTestData(0, 22, "C"));
    if (dinkVoid_) {
      addSV(bv, vui_pid, 3, mvs_pid, 33);
    }
    bm = make_unique<AssnsABX_t>(*b);
    bvm = make_unique<AssnsVoid_t>(*bv);
    addS(bm, bvm, vui_pid, 1, mvs_pid, 0, AssnTestData(1, 0, "AA"));
    if (dinkVoid_) {
      addSV(bvm, vui_pid, 3, mvs_pid, 33);
    }
  }

  e.put(std::move(vui));
  e.put(std::move(vs));
  if (wantMV_) {
    e.put(std::move(mvs), "mv");
    e.put(std::move(b), "mapvec");
    if (wantMany_) {
      e.put(std::move(bm), "manymapvec");
    }
    if (wantVoid_ != "NONE") {
      e.put(std::move(bv), "mapvec");
      if (wantMany_) {
        e.put(std::move(bvm), "manymapvec");
      }
    }
  }
  e.put(std::move(a));
  if (wantMany_) {
    e.put(std::move(am), "many");
  }
  if (wantVoid_ == "ALL") {
    e.put(std::move(av));
    if (wantMany_) {
      e.put(std::move(avm), "many");
    }
  }
  if (wantAmbiguous_) {
    e.put(std::move(ay));
  }
}

DEFINE_ART_MODULE(arttest::AssnsProducer)
