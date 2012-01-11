////////////////////////////////////////////////////////////////////////
// Class:       AssnsAnalyzer
// Module Type: analyzer
// File:        AssnsAnalyzer_module.cc
//
// Generated at Wed Jul 13 14:36:05 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/FindMany.h"
#include "art/Framework/Core/FindManyP.h"
#include "art/Framework/Core/FindOne.h"
#include "art/Framework/Core/FindOneP.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cetlib/maybe_ref.h"
#include "cpp0x/type_traits"
#include "test/TestObjects/AssnTestData.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/type_traits.hpp>

namespace arttest {
   class AssnsAnalyzer;
}

class arttest::AssnsAnalyzer : public art::EDAnalyzer {
public:
  explicit AssnsAnalyzer(fhicl::ParameterSet const & p);
  virtual ~AssnsAnalyzer();

  virtual void analyze(art::Event const & e);

private:
  template <template <typename, typename> class FO>
  void testOne(art::Event const & e) const;

  template <template <typename, typename> class FM>
  void testMany(art::Event const & e) const;

  std::string inputLabel_;
  bool testAB_;
  bool testBA_;
};

namespace {
  typedef size_t A_t;
  typedef std::string B_t;
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> AssnsAB_t;
  typedef art::Assns<std::string, size_t, arttest::AssnTestData> AssnsBA_t;
  typedef art::Assns<size_t, std::string> AssnsABV_t;
  typedef art::Assns<std::string, size_t> AssnsBAV_t;

  // Common case, can dereference (eg Ptr<T>).
  template <typename T, template <typename> class WRAP>
  typename std::enable_if<boost::has_dereference<WRAP<T> >::value, T const &>::type
  dereference(WRAP<T> const & wrapper) {
     return *wrapper;
  }

  // maybe_ref<T>.
  template <typename T, template <typename> class WRAP>
  typename std::enable_if<std::is_same<WRAP<T>, cet::maybe_ref<T> >::value, T const &>::type
  dereference(WRAP<T> const & wrapper) {
     return wrapper.ref();
  }
}

arttest::AssnsAnalyzer::
AssnsAnalyzer(fhicl::ParameterSet const & p)
  :
  inputLabel_(p.get<std::string>("input_label")),
  testAB_(p.get<bool>("test_AB", true)),
  testBA_(p.get<bool>("test_BA", false))
{
}

arttest::AssnsAnalyzer::
~AssnsAnalyzer()
{
}

void
arttest::AssnsAnalyzer::analyze(art::Event const & e)
{
  testOne<art::FindOne>(e);
  testOne<art::FindOneP>(e);
  testMany<art::FindMany>(e);
  testMany<art::FindManyP>(e);
}

template <template <typename, typename> class FO>
void
arttest::AssnsAnalyzer::
testOne(art::Event const & e) const
{
  static char const * const x[] = { "zero", "one", "two" };
  static char const * const a[] = { "A", "B", "C" };
  static size_t const ai[] = { 2, 0, 1 }; // Order in Acoll
  static size_t const bi[] = { 1, 2, 0 }; // Order in Bcoll
  art::Handle<AssnsAB_t> hAB;
  art::Handle<AssnsBA_t> hBA;
  art::Handle<AssnsABV_t> hABV;
  art::Handle<AssnsBAV_t> hBAV;
  if (testAB_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hAB));
    BOOST_REQUIRE_EQUAL(hAB->size(), 3u);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hABV));
    BOOST_REQUIRE_EQUAL(hABV->size(), 3u);
  }
  if (testBA_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBA));
    BOOST_REQUIRE_EQUAL(hBA->size(), 3u);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBAV));
    BOOST_REQUIRE_EQUAL(hBAV->size(), 3u);
  }
  // Construct a FO using a handle to a collection.
  art::Handle<std::vector<size_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));
  FO<std::string, arttest::AssnTestData> foB(hAcoll, e, inputLabel_);
  FO<std::string, void> foBV(hAcoll, e, inputLabel_);
  art::Handle<std::vector<std::string> > hBcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hBcoll));
  FO<size_t, arttest::AssnTestData> foA(hBcoll, e, inputLabel_);
  FO<size_t, void> foAV(hBcoll, e, inputLabel_);
  for (size_t i = 0; i < 3; ++i) {
    if (testAB_) {
      BOOST_CHECK_EQUAL(*(*hAB)[i].first, i);
      BOOST_CHECK_EQUAL(*(*hAB)[i].second, std::string(x[i]));
      BOOST_CHECK_EQUAL((*hAB).data(i).d1, (*hAB)[i].first.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).d2, (*hAB)[i].second.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).label, std::string(a[i]));
      BOOST_CHECK_EQUAL(*(*hABV)[i].first, i);
      BOOST_CHECK_EQUAL(*(*hABV)[i].second, std::string(x[i]));
    }
    if (testBA_) {
      BOOST_CHECK_EQUAL(*(*hBA)[i].first, std::string(x[i]));
      BOOST_CHECK_EQUAL(*(*hBA)[i].second, i);
      BOOST_CHECK_EQUAL((*hBA).data(i).d2, (*hBA)[i].first.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).d1, (*hBA)[i].second.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).label, std::string(a[i]));
      BOOST_CHECK_EQUAL(*(*hBAV)[i].first, std::string(x[i]));
      BOOST_CHECK_EQUAL(*(*hBAV)[i].second, i);
    }
    // Check FindOne.
    BOOST_CHECK_EQUAL(dereference(foB.at(i)), std::string(x[ai[i]]));
    BOOST_CHECK_EQUAL(dereference(foB.data(i)).d1, i);
    BOOST_CHECK_EQUAL(dereference(foB.data(i)).d2, bi[i]);
    BOOST_CHECK_EQUAL(dereference(foBV.at(i)), std::string(x[ai[i]]));
    BOOST_CHECK_EQUAL(dereference(foA.at(i)), bi[i]);
    BOOST_CHECK_EQUAL(dereference(foA.data(i)).d1, ai[i]);
    BOOST_CHECK_EQUAL(dereference(foA.data(i)).d2, i);
    BOOST_CHECK_EQUAL(dereference(foAV.at(i)), bi[i]);
  }
  // Check alternative accessors and range checking for Assns.
  if (testAB_) {
    BOOST_CHECK_THROW((*hAB).at(3), std::out_of_range);
    BOOST_CHECK_EQUAL(&(*hAB).data(0), &(*hAB).data((*hAB).begin()));
    BOOST_CHECK_THROW((*hAB).data(3), std::out_of_range);
  }
  if (testBA_) {
    BOOST_CHECK_THROW((*hBA).at(3), std::out_of_range);
    BOOST_CHECK_EQUAL(&(*hBA).data(0), &(*hBA).data((*hBA).begin()));
    BOOST_CHECK_THROW((*hBA).data(3), std::out_of_range);
  }
  // Check FindOne on View.
  art::View<A_t> va;
  BOOST_REQUIRE(e.getView(inputLabel_, va));
  FO<B_t, arttest::AssnTestData> foBv(va, e, inputLabel_);
  BOOST_REQUIRE(foB == foBv);
  art::View<B_t> vb;
  BOOST_REQUIRE(e.getView(inputLabel_, vb));
  FO<A_t, arttest::AssnTestData> foAv(vb, e, inputLabel_);
  BOOST_REQUIRE(foA == foAv);
  // Check FindOne on PtrVector.
  va.vals()[1] = 0;
  art::PtrVector<A_t> pva;
  va.fill(pva);
  FO<B_t, arttest::AssnTestData> foBpv(pva, e, inputLabel_);
  BOOST_CHECK_EQUAL(foBpv.at(0), foB.at(0));
  BOOST_CHECK_EQUAL(foBpv.data(0), foB.data(0));
  BOOST_CHECK_EQUAL(foBpv.at(1), foB.at(2)); // Knocked out the middle.
  BOOST_CHECK_EQUAL(foBpv.data(1), foB.data(2));
  vb.vals()[1] = 0;
  art::PtrVector<B_t> pvb;
  vb.fill(pvb);
  FO<A_t, arttest::AssnTestData> foApv(pvb, e, inputLabel_);
  BOOST_CHECK_EQUAL(foApv.at(0), foA.at(0));
  BOOST_CHECK_EQUAL(foApv.data(0), foA.data(0));
  BOOST_CHECK_EQUAL(foApv.at(1), foA.at(2)); // Knocked out the middle.
  BOOST_CHECK_EQUAL(foApv.data(1), foA.data(2));
  // Check for range errors.
  BOOST_CHECK_THROW(foApv.at(3), std::out_of_range);
  BOOST_CHECK_THROW(foApv.data(3), std::out_of_range);
  BOOST_CHECK_THROW((FO<B_t, arttest::AssnTestData> (hAcoll, e, art::InputTag(inputLabel_, "M"))), art::Exception);
}

template <template <typename, typename> class FM>
void
arttest::AssnsAnalyzer::
testMany(art::Event const & e) const
{
  art::Handle<std::vector<size_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));
  // Check FindMany.
  FM<B_t, arttest::AssnTestData> fmB(hAcoll, e, art::InputTag(inputLabel_, "M"));
  BOOST_CHECK_EQUAL(fmB.at(0).size(), 1u);
  BOOST_CHECK_EQUAL(fmB.at(1).size(), 2u);
  BOOST_CHECK_EQUAL(fmB.at(2).size(), 1u);
  BOOST_CHECK_EQUAL(fmB.data(0).size(), 1u);
  BOOST_CHECK_EQUAL(fmB.data(1).size(), 2u);
  BOOST_CHECK_EQUAL(fmB.data(2).size(), 1u);
  FM<B_t, void> fmBV(hAcoll, e, art::InputTag(inputLabel_, "M"));
  BOOST_CHECK_EQUAL(fmBV.at(0).size(), 1u);
  BOOST_CHECK_EQUAL(fmBV.at(1).size(), 2u);
  BOOST_CHECK_EQUAL(fmBV.at(2).size(), 1u);
}

DEFINE_ART_MODULE(arttest::AssnsAnalyzer)
