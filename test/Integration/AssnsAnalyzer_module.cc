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
  bool testAB_;
  bool testBA_;
};

namespace {
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> AssnsAB_t;
  typedef art::Assns<std::string, size_t, arttest::AssnTestData> AssnsBA_t;
}

AssnsAnalyzer::AssnsAnalyzer(fhicl::ParameterSet const &p)
  :
  inputLabel_(p.get<std::string>("input_label")),
  testAB_(p.get<bool>("test_AB", true)),
  testBA_(p.get<bool>("test_BA", false))
{
}

AssnsAnalyzer::~AssnsAnalyzer() {
}

void AssnsAnalyzer::analyze(art::Event const &e) {
  char const *x[] = { "zero", "one", "two" };
  char const *a[] = { "A", "B", "C" };

  art::Handle<AssnsAB_t> hAB;
  art::Handle<AssnsBA_t> hBA;

  if (testAB_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hAB));
    BOOST_REQUIRE_EQUAL(hAB->size(),3u);
  }
  if (testBA_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBA));
    BOOST_REQUIRE_EQUAL(hBA->size(),3u);
  }

  for (size_t i = 0; i < 3; ++i) {
    if (testAB_) {
      BOOST_CHECK_EQUAL(*(*hAB)[i].first, i);
      BOOST_CHECK_EQUAL(*(*hAB)[i].second, std::string(x[i]));
      BOOST_CHECK_EQUAL((*hAB).data(i).d1,(*hAB)[i].first.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).d2,(*hAB)[i].second.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).label,std::string(a[i]));
    }
    if (testBA_) {
      BOOST_CHECK_EQUAL(*(*hBA)[i].first, std::string(x[i]));
      BOOST_CHECK_EQUAL(*(*hBA)[i].second, i);
      BOOST_CHECK_EQUAL((*hBA).data(i).d2,(*hBA)[i].first.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).d1,(*hBA)[i].second.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).label,std::string(a[i]));
    }
  }
}

DEFINE_ART_MODULE(AssnsAnalyzer);
