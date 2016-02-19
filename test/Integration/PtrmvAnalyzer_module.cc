////////////////////////////////////////////////////////////////////////
// Class:       PtrmvAnalyzer
// Module Type: analyzer
// File:        PtrmvAnalyzer_module.cc
//
// Generated at Tue May 31 08:01:04 2011 by Chris Green using artmod
// from art v0_07_07.
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/quiet_unit_test.hpp"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"

#include "cetlib/map_vector.h"

#include <string>

namespace arttest {
  class PtrmvAnalyzer;
}

namespace {
  typedef cet::map_vector<std::string> mv_t;
  typedef typename mv_t::value_type mvp_t;
}

class arttest::PtrmvAnalyzer : public art::EDAnalyzer {
public:
  explicit PtrmvAnalyzer(fhicl::ParameterSet const &p);
  virtual ~PtrmvAnalyzer();

  virtual void analyze(art::Event const &e);

private:
  std::string inputLabel_;
};


arttest::PtrmvAnalyzer::PtrmvAnalyzer(fhicl::ParameterSet const &p)
  :
  art::EDAnalyzer(p),
  inputLabel_(p.get<std::string>("input_label"))
{
}

arttest::PtrmvAnalyzer::~PtrmvAnalyzer() {
}

void arttest::PtrmvAnalyzer::analyze(art::Event const &e) {
  // map_vector retrieval.
  art::Handle<mv_t> mv;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, mv));
  std::string const *item;
  item = mv->getOrNull(cet::map_vector_key(0));
  BOOST_CHECK_EQUAL(*item, "ONE");
  item = mv->getOrNull(cet::map_vector_key(3));
  BOOST_CHECK_EQUAL(*item, "TWO");
  item = mv->getOrNull(cet::map_vector_key(5));
  BOOST_CHECK_EQUAL(*item, "THREE");
  item = mv->getOrNull(cet::map_vector_key(7));
  BOOST_CHECK_EQUAL(*item, "FOUR");
  item = mv->getOrNull(cet::map_vector_key(9));
  BOOST_CHECK(item == nullptr);// Not using EQUAL to avoid stream badness.

  // Ptr<std::string> retrieval.
  art::Handle<art::Ptr<std::string> > ptr;
  assert(e.getByLabel(inputLabel_, ptr));
  assert(**ptr == "TWO");

  // PtrVector<std::string> retrieval.
  art::Handle<art::PtrVector<std::string> > pv;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, pv));
  BOOST_CHECK_EQUAL(*(*pv)[0], "THREE");
  BOOST_CHECK_EQUAL(*(*pv)[1], "ONE");
  BOOST_CHECK_EQUAL(*(*pv)[2], "FOUR");
  BOOST_CHECK_EQUAL(*(*pv)[3], "TWO");

  // Ptr<std::string> retrieval.
  art::Handle<art::Ptr<mvp_t> > ptr_p;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, ptr_p));
  BOOST_CHECK_EQUAL((*ptr_p)->first, cet::map_vector_key(3));
  BOOST_CHECK_EQUAL((*ptr_p)->second, "TWO");

  // PtrVector<std::string> retrieval.
  art::Handle<art::PtrVector<mvp_t> > pvp;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, pvp));
  BOOST_CHECK_EQUAL((*pvp)[0]->first, cet::map_vector_key(5));
  BOOST_CHECK_EQUAL((*pvp)[0]->second, "THREE");
  BOOST_CHECK_EQUAL((*pvp)[1]->first, cet::map_vector_key(0));
  BOOST_CHECK_EQUAL((*pvp)[1]->second, "ONE");
  BOOST_CHECK_EQUAL((*pvp)[2]->first, cet::map_vector_key(7));
  BOOST_CHECK_EQUAL((*pvp)[2]->second, "FOUR");
  BOOST_CHECK_EQUAL((*pvp)[3]->first, cet::map_vector_key(3));
  BOOST_CHECK_EQUAL((*pvp)[3]->second, "TWO");
}

DEFINE_ART_MODULE(arttest::PtrmvAnalyzer)
