////////////////////////////////////////////////////////////////////////
// Class:       PtrVectorSimpleAnalyzer
// Module Type: analyzer
// File:        PtrVectorSimpleAnalyzer_module.cc
//
// Generated at Fri Oct  5 15:02:19 2012 by Chris Green using artmod
// from art v1_02_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
  class PtrVectorSimpleAnalyzer;
}

class arttest::PtrVectorSimpleAnalyzer : public art::SharedAnalyzer {
public:
  struct Config {
    fhicl::Atom<std::string> input_label{fhicl::Name{"input_label"}};
  };
  using Parameters = Table<Config>;
  explicit PtrVectorSimpleAnalyzer(Parameters const& p);

private:
  void analyze(art::Event const& e, art::Services const&) override;

  std::string const input_label_;
};

arttest::PtrVectorSimpleAnalyzer::PtrVectorSimpleAnalyzer(Parameters const& p)
  : art::SharedAnalyzer{p}, input_label_{p().input_label()}
{
  async<art::InEvent>();
}

void
arttest::PtrVectorSimpleAnalyzer::analyze(art::Event const& e,
                                          art::Services const&)
{
  using SimplePtrVector = art::PtrVector<arttest::Simple>;

  auto p = e.getPointerByLabel<SimplePtrVector>(input_label_);
  assert(p);
  assert(p->size() > 0);
  // assert( p->pop_back()); // This fails to compile, because *p is const.

  art::Handle<SimplePtrVector> h;
  bool const status = e.getByLabel(input_label_, h);
  assert(status);

  int const event_num = e.id().event();
  size_t const sz = h->size();
  size_t i = 0;
  for (auto const ptr : *h) {
    assert((unsigned)ptr->key == sz - i + event_num);
    double const expect = 1.5 * i + 100.0;
    assert(ptr->value == expect);
    assert(ptr->dummy() == 16.25);
    ++i;
  }
}

DEFINE_ART_MODULE(arttest::PtrVectorSimpleAnalyzer)
