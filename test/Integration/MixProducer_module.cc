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
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
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

  // Declare member data here.
  size_t eventCounter;

};


arttest::MixProducer::MixProducer(fhicl::ParameterSet const &p)
  :
  eventCounter(0)
{
  produces<double>("doubleLabel");
  produces<IntProduct>("IntProductLabel");
  produces<std::string>("stringLabel");
  produces<std::vector<double> >("doubleCollectionLabel");
  produces<std::vector<art::Ptr<double> > >("doubleVectorPtrLabel");
  produces<art::PtrVector<double> >("doublePtrVectorLabel");
  produces<ProductWithPtrs>("ProductWithPtrsLabel");
}

arttest::MixProducer::~MixProducer() {
  // Clean up dynamic memory and other resources here.
}

void arttest::MixProducer::produce(art::Event &e) {
  ++eventCounter;

  // double
  e.put(std::auto_ptr<double>(new double(eventCounter)), "doubleLabel");

  // IntProduct
  e.put(std::auto_ptr<IntProduct>(new IntProduct(eventCounter + 1000000)), "IntProductLabel");

  // std::string
  std::ostringstream s;
  s << "string value: " << std::setfill('0') << std::setw(7) << eventCounter;
  e.put(std::auto_ptr<std::string>(new std::string(s.str())), "stringLabel");

  // 1. std::vector<double>
  //
  // 2. std::vector<art::Ptr<double> >
  //
  // 3. art::PtrVector<double>
  //
  // 4. ProductWithPtrs
  std::auto_ptr<std::vector<double> > coll(new std::vector<double>(10));
  for (size_t i = 1; i < 11; ++i) {
    coll->push_back(i + 10 * eventCounter);
  }
  art::OrphanHandle<std::vector<double> >
    collHandle(e.put(coll, "doubleCollectionLabel")); // 1.
  std::auto_ptr<std::vector<art::Ptr<double> > >
    vpd(new std::vector<art::Ptr<double> >(3));
  std::auto_ptr<art::PtrVector<double> >
    pvd(new art::PtrVector<double>());
  pvd->reserve(3);
  vpd->push_back(art::Ptr<double>(collHandle, 0));
  vpd->push_back(art::Ptr<double>(collHandle, 4));
  vpd->push_back(art::Ptr<double>(collHandle, 8));
  pvd->push_back(art::Ptr<double>(collHandle, 1));
  pvd->push_back(art::Ptr<double>(collHandle, 5));
  pvd->push_back(art::Ptr<double>(collHandle, 9));
  std::auto_ptr<ProductWithPtrs> pwp(new ProductWithPtrs(*pvd.get(), *vpd.get()));
  e.put(vpd, "doubleVectorPtrLabel"); // 2.
  e.put(pvd, "doublePtrVectorLabel"); // 3.
  e.put(pwp, "ProductWithPtrsLabel"); // 4.
}

DEFINE_ART_MODULE(arttest::MixProducer);
