////////////////////////////////////////////////////////////////////////
// Class:       MixAnalyzer
// Module Type: analyzer
// File:        MixAnalyzer_module.cc
//
// Generated at Mon May 16 10:45:57 2011 by Chris Green using artmod
// from art v0_06_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>

namespace arttest {
  class MixAnalyzer;
}

class arttest::MixAnalyzer : public art::EDAnalyzer {
public:
  explicit MixAnalyzer(fhicl::ParameterSet const &p);
  virtual ~MixAnalyzer();

  virtual void analyze(art::Event const &e);


private:

  size_t eventCounter_;
  size_t nSecondaries_;
  std::string mixFilterLabel_;
  
};


arttest::MixAnalyzer::MixAnalyzer(fhicl::ParameterSet const &p)
  :
  eventCounter_(0),
  nSecondaries_(p.get<size_t>("numSecondaries", 1)),
  mixFilterLabel_(p.get<std::string>("mixFilterLabel", "mixFilter"))
{
}

arttest::MixAnalyzer::~MixAnalyzer() {
  // Clean up dynamic memory and other resources here.
}

void arttest::MixAnalyzer::analyze(art::Event const &e) {
  ++eventCounter_;

  // Double
  art::Handle<double> dH;
  assert(e.getByLabel(mixFilterLabel_, "doubleLabel", dH));
  double dExpected = ((2 * eventCounter_ - 1) * nSecondaries_ + 1) * nSecondaries_ / 2;
  assert(*dH ==  dExpected);

  // IntProduct
  art::Handle<IntProduct> ipH;
  assert(e.getByLabel(mixFilterLabel_, "IntProductLabel", ipH));
  assert(ipH->value == dExpected + 1000000 * nSecondaries_); 

  // String
  art::Handle<std::string> sH;
  assert(e.getByLabel(mixFilterLabel_, "stringLabel", sH));
  std::ostringstream sExp;
  for (size_t i = 1; i <= nSecondaries_; ++i) {
    sExp << "string value: " << std::setfill('0') << std::setw(7) << (eventCounter_ - 1) * nSecondaries_ + i << "\n";
  }
  assert(*sH == sExp.str());

  // 1. std::vector<double>
  art::Handle<std::vector<double> > vdH;
  assert(e.getByLabel(mixFilterLabel_, "doubleCollectionLabel", vdH));
  assert(vdH->size() == 10 * nSecondaries_);
  for (std::vector<double>::const_iterator
         i = vdH->begin(),
         end = vdH->end();
       i != end;
       ++i) {
  }
  for (size_t i = 0; i < nSecondaries_; ++i) {
    for (size_t j = 1; j < 11; ++j) {
      assert((*vdH)[i*10 + j - 1] == j + 10 * (i + (eventCounter_ - 1) * nSecondaries_));
    }
  }

  // 2. std::vector<art::Ptr<double> >
  // 3. art::PtrVector<double>
  // 4. ProductWithPtrs
  art::Handle<std::vector<art::Ptr<double> > > vpdH;
  assert(e.getByLabel(mixFilterLabel_, "doubleVectorPtrLabel", vpdH)); // 2.
#ifndef ART_NO_MIX_PTRVECTOR
  art::Handle<art::PtrVector<double> > pvdH;
  assert(e.getByLabel(mixFilterLabel_, "doubleVectorPtrLabel", vpdH)); // 3.
#endif
  art::Handle<arttest::ProductWithPtrs> pwpH;
  assert(e.getByLabel(mixFilterLabel_, "ProductWithPtrsLabel", pwpH)); // 4.

  for (size_t i = 0; i < nSecondaries_; ++i) {
    assert(*(*vpdH)[i * 3 + 0] == (*vdH)[(i * 10) + 0]); // 2.
    assert(*(*vpdH)[i * 3 + 1] == (*vdH)[(i * 10) + 4]); // 2.
    assert(*(*vpdH)[i * 3 + 2] == (*vdH)[(i * 10) + 8]); // 2.
#ifndef ART_NO_MIX_PTRVECTOR
    assert(*(*pvdH)[i * 3 + 0] == (*vdH)[(i * 10) + 1]); // 3.
    assert(*(*pvdH)[i * 3 + 1] == (*vdH)[(i * 10) + 5]); // 3.
    assert(*(*pvdH)[i * 3 + 2] == (*vdH)[(i * 10) + 9]); // 3.
#endif
    assert(*(pwpH->vectorPtrDouble())[i * 3 + 0] == *(*vpdH)[i * 3 + 0]); // 4.
    assert(*(pwpH->vectorPtrDouble())[i * 3 + 1] == *(*vpdH)[i * 3 + 1]); // 4.
    assert(*(pwpH->vectorPtrDouble())[i * 3 + 2] == *(*vpdH)[i * 3 + 2]); // 4.
#ifndef ART_NO_MIX_PTRVECTOR
    assert(*(pwpH->ptrVectorDouble())[i * 3 + 0] == *(*pvdH)[i * 3 + 0]); // 4.
    assert(*(pwpH->ptrVectorDouble())[i * 3 + 1] == *(*pvdH)[i * 3 + 1]); // 4.
    assert(*(pwpH->ptrVectorDouble())[i * 3 + 2] == *(*pvdH)[i * 3 + 2]); // 4.
#endif
  }

  // Bookkeeping.
  art::Handle<std::string> bsH;
  assert(e.getByLabel(mixFilterLabel_, bsH));
  assert((*bsH) == "BlahBlahBlah");

  art::Handle<art::EventIDSequence> eidsH;
  assert(e.getByLabel(mixFilterLabel_, eidsH));
  for (size_t i = 0; i < nSecondaries_; ++i) {
    assert((*eidsH)[i].event() == (eventCounter_ - 1) * nSecondaries_ + i + 1);
  }
}

DEFINE_ART_MODULE(arttest::MixAnalyzer);
