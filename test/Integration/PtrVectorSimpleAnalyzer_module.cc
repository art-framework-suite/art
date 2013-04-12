////////////////////////////////////////////////////////////////////////
// Class:       PtrVectorSimpleAnalyzer
// Module Type: analyzer
// File:        PtrVectorSimpleAnalyzer_module.cc
//
// Generated at Fri Oct  5 15:02:19 2012 by Chris Green using artmod
// from art v1_02_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  class PtrVectorSimpleAnalyzer;
}

class arttest::PtrVectorSimpleAnalyzer : public art::EDAnalyzer {
public:
  explicit PtrVectorSimpleAnalyzer(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;
private:
  std::string input_label_;
};


arttest::PtrVectorSimpleAnalyzer::PtrVectorSimpleAnalyzer(fhicl::ParameterSet const & p)
  :
  input_label_(p.get<std::string>("input_label"))
{
}

void arttest::PtrVectorSimpleAnalyzer::analyze(art::Event const & e)
{
  typedef art::PtrVector<arttest::Simple> SimplePtrVector;

  auto p = e.getPointerByLabel<SimplePtrVector>(input_label_);
  assert(p);
  assert( p->size() > 0);
  //assert( p->pop_back()); // This fails to compile, because *p is const.

  art::Handle<SimplePtrVector> h;
  bool status = e.getByLabel(input_label_, h);
  assert(status);

  int const event_num = e.id().event();
  for (size_t i = 0, sz = h->size(); i != sz; ++i) {
    assert((unsigned)(*h)[i]->key == sz - i + event_num);
    double const expect = 1.5 * i + 100.0;
    assert((*h)[i]->value == expect);
    assert((*h)[i]->dummy() == 16.25);
  }
}

DEFINE_ART_MODULE(arttest::PtrVectorSimpleAnalyzer)
