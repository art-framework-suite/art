////////////////////////////////////////////////////////////////////////
// Class:       PtrMakerProducer2
// Plugin Type: producer (art v2_05_00)
// File:        PtrMakerProducer2_module.cc
//
// Generated at Wed Nov 23 22:59:41 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/PtrMaker.h"
#include "fhiclcpp/types/Atom.h"

#include <cassert>
#include <memory>

class PtrMakerProducer2 : public art::EDProducer {
public:
  using intvector_t = std::vector<int>;
  using intPtrvector_t = art::PtrVector<int>;

  struct Config {
    fhicl::Atom<unsigned> nvalues{fhicl::Name{"nvalues"}};
  };
  using Parameters = art::EDProducer::Table<Config>;
  explicit PtrMakerProducer2(Parameters const& p);

  // Plugins should not be copied or assigned.
  PtrMakerProducer2(PtrMakerProducer2 const&) = delete;
  PtrMakerProducer2(PtrMakerProducer2&&) = delete;
  PtrMakerProducer2& operator=(PtrMakerProducer2 const&) = delete;
  PtrMakerProducer2& operator=(PtrMakerProducer2&&) = delete;

private:
  void produce(art::Event& e) override;
  unsigned const nvalues_;
};

PtrMakerProducer2::PtrMakerProducer2(Parameters const& p)
  : nvalues_{p().nvalues()}
{
  produces<intvector_t>();
  produces<intPtrvector_t>();
}

void
PtrMakerProducer2::produce(art::Event& e)
{
  auto const value = e.id().event();
  auto intvector = std::make_unique<intvector_t>();
  auto intptrs = std::make_unique<intPtrvector_t>();

  // Make two PtrMakers--where one of them uses the named-constructor
  // idiom ('create'), explicitly specifying the desired type.  The
  // 'create' function is necessary if a collection other than
  // std::vector is desired.
  art::PtrMaker<int> const make_intptr1{e, *this};
  auto const make_intptr2 =
    art::PtrMaker<int>::create<std::vector<int>>(e, *this);

  for (unsigned i{}; i != nvalues_; ++i) {
    intvector->push_back(value + i);
    auto p1 = make_intptr1(i);
    auto p2 = make_intptr2(i);
    // Ensure that the two art::Ptrs are equal, but only persist one
    // copy.
    assert(p1 == p2);
    intptrs->push_back(p1);
  }
  e.put(std::move(intvector));
  e.put(std::move(intptrs));
}

DEFINE_ART_MODULE(PtrMakerProducer2)
