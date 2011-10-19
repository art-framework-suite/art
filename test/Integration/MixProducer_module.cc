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
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cetlib/map_vector.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace arttest {
  class MixProducer;
}

class arttest::MixProducer : public art::EDProducer {
public:
  explicit MixProducer(fhicl::ParameterSet const &p);
  virtual ~MixProducer();

  virtual void produce(art::Event &e);

private:
  typedef cet::map_vector<unsigned int> mv_t;
  typedef typename mv_t::value_type mvv_t;
  typedef typename mv_t::mapped_type mvm_t;

  // Declare member data here.
  size_t eventCounter_;

};


arttest::MixProducer::MixProducer(fhicl::ParameterSet const &)
  :
  eventCounter_(0)
{
  produces<double>("doubleLabel");
  produces<IntProduct>("IntProductLabel");
  produces<std::string>("stringLabel");
  produces<std::vector<double> >("doubleCollectionLabel");
  produces<std::vector<art::Ptr<double> > >("doubleVectorPtrLabel");
  produces<art::PtrVector<double> >("doublePtrVectorLabel");
  produces<ProductWithPtrs>("ProductWithPtrsLabel");
  produces<mv_t>("mapVectorLabel");
  produces<std::vector<art::Ptr<mvv_t> > >("intVectorPtrLabel");
}

arttest::MixProducer::~MixProducer() {
  // Clean up dynamic memory and other resources here.
}

void arttest::MixProducer::produce(art::Event &e) {
  ++eventCounter_;

  // double
  e.put(std::auto_ptr<double>(new double(eventCounter_)), "doubleLabel");

  // IntProduct
  e.put(std::auto_ptr<IntProduct>(new IntProduct(eventCounter_ + 1000000)), "IntProductLabel");

  // std::string
  std::ostringstream s;
  s << "string value: " << std::setfill('0') << std::setw(7) << eventCounter_ << "\n";
  e.put(std::auto_ptr<std::string>(new std::string(s.str())), "stringLabel");

  // 1. std::vector<double>
  //
  // 2. std::vector<art::Ptr<double> >
  //
  // 3. art::PtrVector<double>
  //
  // 4. ProductWithPtrs
  std::auto_ptr<std::vector<double> > coll(new std::vector<double>);
  coll->reserve(10);
  for (size_t i = 1; i < 11; ++i) {
    coll->push_back(i + 10 * (eventCounter_ - 1));
  }
  e.put(coll, "doubleCollectionLabel"); // 1.
  std::auto_ptr<std::vector<art::Ptr<double> > >
    vpd(new std::vector<art::Ptr<double> >);
  vpd->reserve(3);
  std::auto_ptr<art::PtrVector<double> >
    pvd(new art::PtrVector<double>());
  pvd->reserve(3);
  art::ProductID collID(getProductID<std::vector<double> >(e, "doubleCollectionLabel"));
  vpd->push_back(art::Ptr<double>(collID, 0, e.productGetter(collID)));
  vpd->push_back(art::Ptr<double>(collID, 4, e.productGetter(collID)));
  vpd->push_back(art::Ptr<double>(collID, 8, e.productGetter(collID)));
  pvd->push_back(art::Ptr<double>(collID, 1, e.productGetter(collID)));
  pvd->push_back(art::Ptr<double>(collID, 5, e.productGetter(collID)));
  pvd->push_back(art::Ptr<double>(collID, 9, e.productGetter(collID)));
  std::auto_ptr<ProductWithPtrs>
    pwp(new ProductWithPtrs(
#ifndef ART_NO_MIX_PTRVECTOR
                            *pvd.get(),
#endif
                            *vpd.get()));
  e.put(vpd, "doubleVectorPtrLabel"); // 2.
  e.put(pvd, "doublePtrVectorLabel"); // 3.
  e.put(pwp, "ProductWithPtrsLabel"); // 4.

  // map_vector, .
  std::auto_ptr<mv_t> mv(new mv_t);
  static size_t const mv_size = 5;
  mv->reserve(mv_size);
  for (size_t i = 0; i < mv_size; ++i) {
    (*mv)[cet::map_vector_key(static_cast<mvm_t>(1 + i * 2 + 10 * (eventCounter_ - 1)))] =
      (eventCounter_ - 1) * mv_size + i + 1;
  }

  // Ptr into map_vector.
  std::auto_ptr<std::vector<art::Ptr<mvv_t> > > mvvp(new std::vector<art::Ptr<mvv_t> >);
  mvvp->reserve(mv_size);
  art::ProductID mvID(getProductID<mv_t>(e, "mapVectorLabel"));
  mvvp->push_back(art::Ptr<mvv_t>(mvID, 10 * (eventCounter_ - 1) + 7, e.productGetter(mvID)));
  mvvp->push_back(art::Ptr<mvv_t>(mvID, 10 * (eventCounter_ - 1) + 1, e.productGetter(mvID)));
  mvvp->push_back(art::Ptr<mvv_t>(mvID, 10 * (eventCounter_ - 1) + 3, e.productGetter(mvID)));
  mvvp->push_back(art::Ptr<mvv_t>(mvID, 10 * (eventCounter_ - 1) + 9, e.productGetter(mvID)));
  mvvp->push_back(art::Ptr<mvv_t>(mvID, 10 * (eventCounter_ - 1) + 5, e.productGetter(mvID)));

  e.put(mvvp, "intVectorPtrLabel");
  e.put(mv, "mapVectorLabel"); // Note we're putting these into the event in the "wrong" order.
}

DEFINE_ART_MODULE(arttest::MixProducer);
