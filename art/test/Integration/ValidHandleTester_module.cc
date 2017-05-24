////////////////////////////////////////////////////////////////////////
// Class:       ValidHandleTester
// Module Type: analyzer
// File:        ValidHandleTester_module.cc
//
// Generated at Thu Apr 11 13:40:02 2013 by Marc Paterno using artmod
// from cetpkgsupport v1_01_00.
////////////////////////////////////////////////////////////////////////

#include <string>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

class ValidHandleTester : public art::EDAnalyzer {

  struct Config {
    fhicl::Atom<std::string> input_label {fhicl::Name{"input_label"}};
    fhicl::Atom<std::string> expected_value {fhicl::Name{"expected_value"}};
  };

public:

  using Parameters = art::EDAnalyzer::Table<Config>;
  explicit ValidHandleTester(Parameters const& p);
  void analyze(art::Event const& e) override;

private:
  art::InputTag input_tag_;
  std::string expected_value_;
};


ValidHandleTester::ValidHandleTester(Parameters const& ps) :
  art::EDAnalyzer{ps},
  input_tag_{ps().input_label()},
  expected_value_{ps().expected_value()}
{
  consumes<std::string>(input_tag_);
}

void ValidHandleTester::analyze(art::Event const& e)
{
  // Make sure old-form access works.
  art::Handle<std::string> h;
  e.getByLabel(input_tag_, h);
  assert(h.isValid());
  assert(*h == expected_value_);

  // Make sure new-form access works.
  auto p = e.getValidHandle<std::string>(input_tag_);
  assert(*p == expected_value_);
  auto const& prov = *p.provenance();
  assert(prov == *h.provenance());

  // Test that Provenance::inputTag() was formed correctly
  auto const& tag = prov.inputTag();
  assert(tag.label() == prov.moduleLabel());
  assert(tag.instance() == prov.productInstanceName());
  assert(tag.process() == prov.processName());

  // Make sure conversion-to-pointer works.
  [&](std::string const* x){ assert(*x == expected_value_); }(p);
}

DEFINE_ART_MODULE(ValidHandleTester)
