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
public:
  explicit ValidHandleTester(fhicl::ParameterSet const & p);
  virtual ~ValidHandleTester();

  void analyze(art::Event const & e) override;

private:
  art::InputTag input_tag_;
  std::string expected_value_;
};


ValidHandleTester::ValidHandleTester(fhicl::ParameterSet const & ps) :
  input_tag_(ps.get<std::string>("input_label")),
  expected_value_(ps.get<std::string>("expected_value"))
{ }

ValidHandleTester::~ValidHandleTester()
{ }

void ValidHandleTester::analyze(art::Event const & e)
{
  // Make sure old-form access works.
  art::Handle<std::string> h;
  e.getByLabel(input_tag_, h);
  assert(h.isValid());
  assert(*h == expected_value_);

  // Make sure new-form access works.
  auto p = e.getValidHandle<std::string>(input_tag_);
  assert(*p == expected_value_);
  assert(*p.provenance() == *h.provenance());
}

DEFINE_ART_MODULE(ValidHandleTester)
