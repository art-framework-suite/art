////////////////////////////////////////////////////////////////////////
// Class:       MixProducer
// Module Type: producer
// File:        MixProducer_module.cc
//
// Generated at Wed May 11 10:14:20 2011 by Chris Green using artmod
// from art v0_06_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ProductWithPtrs.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "cetlib/map_vector.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace arttest {
  class MixProducer;
}

class arttest::MixProducer : public art::EDProducer {
public:
  explicit MixProducer(fhicl::ParameterSet const& p);

  void produce(art::Event& e) override;
  void endSubRun(art::SubRun& sr) override;
  void endRun(art::Run& r) override;

private:
  using mv_t = cet::map_vector<unsigned int>;
  using mvv_t = mv_t::value_type;
  using mvm_t = mv_t::mapped_type;

  // Declare member data here.
  size_t eventCounter_{};
  size_t subrunCounter_{};
  size_t runCounter_{};
};

arttest::MixProducer::MixProducer(fhicl::ParameterSet const&)
{
  produces<double>("doubleLabel");
  produces<IntProduct>("IntProductLabel");
  produces<IntProduct>("SpottyProductLabel");
  produces<std::string>("stringLabel");
  produces<std::vector<double>>("doubleCollectionLabel");
  produces<std::vector<art::Ptr<double>>>("doubleVectorPtrLabel");
  produces<art::PtrVector<double>>("doublePtrVectorLabel");
  produces<ProductWithPtrs>("ProductWithPtrsLabel");
  produces<mv_t>("mapVectorLabel");
  produces<std::vector<art::Ptr<mvv_t>>>("intVectorPtrLabel");
  produces<double, art::InSubRun>("DoubleSRLabel");
  produces<double, art::InRun>("DoubleRLabel");
}

void
arttest::MixProducer::produce(art::Event& e)
{
  ++eventCounter_;

  // double
  e.put(std::make_unique<double>(eventCounter_), "doubleLabel");

  // IntProduct
  e.put(std::make_unique<IntProduct>(eventCounter_ + 1000000),
        "IntProductLabel");

  // SpottyProduct
  if (e.event() % 100) {
    e.put(std::make_unique<IntProduct>(eventCounter_), "SpottyProductLabel");
  }

  // std::string
  std::ostringstream s;
  s << "string value: " << std::setfill('0') << std::setw(7) << eventCounter_
    << "\n";
  e.put(std::make_unique<std::string>(s.str()), "stringLabel");

  // 1. std::vector<double>
  //
  // 2. std::vector<art::Ptr<double>>
  //
  // 3. art::PtrVector<double>
  //
  // 4. ProductWithPtrs
  auto coll = std::make_unique<std::vector<double>>();
  coll->reserve(10);
  for (size_t i = 1; i < 11; ++i) {
    coll->push_back(i + 10 * (eventCounter_ - 1));
  }
  e.put(move(coll), "doubleCollectionLabel"); // 1.
  auto vpd = std::make_unique<std::vector<art::Ptr<double>>>();
  vpd->reserve(3);
  auto pvd = std::make_unique<art::PtrVector<double>>();
  pvd->reserve(3);
  art::ProductID const collID{
    e.getProductID<std::vector<double>>("doubleCollectionLabel")};
  vpd->emplace_back(collID, 0, e.productGetter(collID));
  vpd->emplace_back(collID, 4, e.productGetter(collID));
  vpd->emplace_back(collID, 8, e.productGetter(collID));
  pvd->push_back(art::Ptr<double>(collID, 1, e.productGetter(collID)));
  pvd->push_back(art::Ptr<double>(collID, 5, e.productGetter(collID)));
  pvd->push_back(art::Ptr<double>(collID, 9, e.productGetter(collID)));
  auto pwp = std::make_unique<ProductWithPtrs>(
#ifndef ART_NO_MIX_PTRVECTOR
    *pvd.get(),
#endif
    *vpd.get());
  e.put(move(vpd), "doubleVectorPtrLabel");      // 2.
  e.put(std::move(pvd), "doublePtrVectorLabel"); // 3.
  e.put(move(pwp), "ProductWithPtrsLabel");      // 4.

  // map_vector, .
  auto mv = std::make_unique<mv_t>();
  static size_t const mv_size = 5;
  mv->reserve(mv_size);
  for (size_t i = 0; i < mv_size; ++i) {
    (*mv)[cet::map_vector_key(
      static_cast<mvm_t>(1 + i * 2 + 10 * (eventCounter_ - 1)))] =
      (eventCounter_ - 1) * mv_size + i + 1;
  }

  // Ptr into map_vector.
  auto mvvp = std::make_unique<std::vector<art::Ptr<mvv_t>>>();
  mvvp->reserve(mv_size);
  art::ProductID const mvID{e.getProductID<mv_t>("mapVectorLabel")};
  mvvp->emplace_back(mvID, 10 * (eventCounter_ - 1) + 7, e.productGetter(mvID));
  mvvp->emplace_back(mvID, 10 * (eventCounter_ - 1) + 1, e.productGetter(mvID));
  mvvp->emplace_back(mvID, 10 * (eventCounter_ - 1) + 3, e.productGetter(mvID));
  mvvp->emplace_back(mvID, 10 * (eventCounter_ - 1) + 9, e.productGetter(mvID));
  mvvp->emplace_back(mvID, 10 * (eventCounter_ - 1) + 5, e.productGetter(mvID));

  e.put(move(mvvp), "intVectorPtrLabel");
  e.put(move(mv), "mapVectorLabel"); // Note we're putting these into the event
                                     // in the "wrong" order.
}

void
arttest::MixProducer::endSubRun(art::SubRun& sr)
{
  ++subrunCounter_;
  sr.put(std::make_unique<double>(subrunCounter_), "DoubleSRLabel");
}

void
arttest::MixProducer::endRun(art::Run& r)
{
  ++runCounter_;
  r.put(std::make_unique<double>(runCounter_), "DoubleRLabel");
}

DEFINE_ART_MODULE(arttest::MixProducer)
