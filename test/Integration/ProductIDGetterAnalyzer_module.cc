////////////////////////////////////////////////////////////////////////
// Class:       ProductIDGetterAnalyzer
// Module Type: analyzer
// File:        ProductIDGetterAnalyzer_module.cc
//
// Generated at Thu Jun 16 11:18:23 2011 by Chris Green using artmod
// from art v0_07_09.
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/quiet_unit_test.hpp"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Ptr.h"

namespace arttest {
  class ProductIDGetterAnalyzer;
}

class arttest::ProductIDGetterAnalyzer : public art::EDAnalyzer {
public:
  explicit ProductIDGetterAnalyzer(fhicl::ParameterSet const &p);
  virtual ~ProductIDGetterAnalyzer();

  virtual void analyze(art::Event const &e);


private:
  std::string input_label_;
};


arttest::ProductIDGetterAnalyzer::ProductIDGetterAnalyzer(fhicl::ParameterSet const &p)
  :
  art::EDAnalyzer(p),
  input_label_(p.get<std::string>("input_label"))
{
}

arttest::ProductIDGetterAnalyzer::~ProductIDGetterAnalyzer() {
}

void arttest::ProductIDGetterAnalyzer::analyze(art::Event const &e) {
  art::Handle<art::Ptr<int> > h;
  BOOST_REQUIRE(e.getByLabel(input_label_, h));

  BOOST_REQUIRE_EQUAL(**h, 4);
}

DEFINE_ART_MODULE(arttest::ProductIDGetterAnalyzer)
