////////////////////////////////////////////////////////////////////////
// Class:       AssnsAnalyzer
// Module Type: analyzer
// File:        AssnsAnalyzer_module.cc
//
// Generated at Wed Jul 13 14:36:05 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/AssnTestData.h"

class AssnsAnalyzer;

class AssnsAnalyzer : public art::EDAnalyzer {
public:
  explicit AssnsAnalyzer(fhicl::ParameterSet const &p);
  virtual ~AssnsAnalyzer();

  virtual void analyze(art::Event const &e);

private:
  std::string inputLabel_;
};

namespace {
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> Assns_t;
}

AssnsAnalyzer::AssnsAnalyzer(fhicl::ParameterSet const &p)
  :
  inputLabel_(p.get<std::string>("input_label"))
{
}

AssnsAnalyzer::~AssnsAnalyzer() {
}

void AssnsAnalyzer::analyze(art::Event const &e) {
  art::Handle<Assns_t> h;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, h));

  BOOST_REQUIRE_EQUAL(h->size(),3);

  char const *x[] = { "zero", "one", "two" };
  char const *a[] = { "A", "B", "C" };

  for (size_t i = 0; i < 3; ++i) {
    BOOST_CHECK_EQUAL(*(*h)[i].first, i);
    BOOST_CHECK_EQUAL(*(*h)[i].second, std::string(x[i]));
    BOOST_CHECK_EQUAL((*h).data(i).d1,(*h)[i].first.key());
    BOOST_CHECK_EQUAL((*h).data(i).d2,(*h)[i].second.key());
    BOOST_CHECK_EQUAL((*h).data(i).label,std::string(a[i]));
  }
}

DEFINE_ART_MODULE(AssnsAnalyzer);
