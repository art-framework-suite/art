////////////////////////////////////////////////////////////////////////
// Class:       DropOnInputTestAnalyzer
// Module Type: analyzer
// File:        DropOnInputTestAnalyzer_module.cc
//
// Generated at Mon Aug  1 13:28:48 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Ptr.h"

#include <string>

namespace arttest {
  class DropOnInputTestAnalyzer;
}

class arttest::DropOnInputTestAnalyzer : public art::EDAnalyzer {
public:
  explicit DropOnInputTestAnalyzer(fhicl::ParameterSet const &p);
  virtual ~DropOnInputTestAnalyzer();

  virtual void analyze(art::Event const &e);


private:
  std::string inputLabel_;
};

arttest::DropOnInputTestAnalyzer::DropOnInputTestAnalyzer(fhicl::ParameterSet const &p)
  :
  inputLabel_(p.get<std::string>("input_label"))
{
}

arttest::DropOnInputTestAnalyzer::~DropOnInputTestAnalyzer() {
}

void arttest::DropOnInputTestAnalyzer::analyze(art::Event const &e) {
  art::Handle<art::Ptr<std::string> > sh;
  BOOST_CHECK(!e.getByLabel(inputLabel_, sh));
  BOOST_REQUIRE(!sh.isValid());
}

DEFINE_ART_MODULE(arttest::DropOnInputTestAnalyzer);
