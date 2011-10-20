////////////////////////////////////////////////////////////////////////
// Class:       DropOnInputTestAnalyzer
// Module Type: analyzer
// File:        DropOnInputTestAnalyzer_module.cc
//
// Generated at Mon Aug  1 13:28:48 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include <boost/test/included/unit_test.hpp>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
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

  typedef cet::map_vector<std::string> mv_t;
  art::Handle<mv_t> mvth;
  BOOST_CHECK(e.getByLabel(inputLabel_, mvth));
  BOOST_REQUIRE(mvth.isValid());
  mv_t const & mapvec = *mvth;
  BOOST_REQUIRE(mapvec[cet::map_vector_key(7)] == "FOUR");
  BOOST_REQUIRE(mapvec[cet::map_vector_key(5)] == "THREE");
  BOOST_REQUIRE(mapvec[cet::map_vector_key(3)] == "TWO");
  BOOST_REQUIRE(mapvec[cet::map_vector_key(0)] == "ONE");
}

DEFINE_ART_MODULE(arttest::DropOnInputTestAnalyzer)
