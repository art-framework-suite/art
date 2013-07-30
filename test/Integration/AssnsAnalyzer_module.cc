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

#include "boost/test/included/unit_test.hpp"
#include "boost/type_traits.hpp"

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
  bool bCollMissing_;
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

  // Test references.
  char const * const x[] = { "zero", "one", "two" };
  char const * const a[] = { "A", "B", "C" };
  size_t const ai[] = { 2, 0, 1 }; // Order in Acoll
  size_t const bi[] = { 1, 2, 0 }; // Order in Bcoll

  template <typename I, typename D, typename F, typename FV>
  void
  check_get_one_impl(F const & fA, FV const & fAV) {
    I item;
    D data;
    for (size_t i = 0; i < 3; ++i) {
      fA.get(i, item, data);
      BOOST_CHECK_EQUAL(dereference(item), bi[i]);
      BOOST_CHECK_EQUAL(dereference(data).d1, ai[i]);
      BOOST_CHECK_EQUAL(dereference(data).d2, i);
      fAV.get(i, item);
      BOOST_CHECK_EQUAL(dereference(item), bi[i]);
    }
  }

  template <typename T, typename D,
            template <typename, typename> class FO>
  typename std::enable_if<std::is_same<FO<T, void>, art::FindOne<T, void> >::value>::type
  check_get(FO<T, D> const & fA,
            FO<T, void> const & fAV) {
    typedef cet::maybe_ref<typename FO<T, void>::assoc_t const> item_t;
    typedef cet::maybe_ref<typename FO<T, D>::data_t const> data_t;
    check_get_one_impl<item_t, data_t>(fA, fAV);
  }

  template <typename T, typename D,
            template <typename, typename> class FO>
  typename std::enable_if<std::is_same<FO<T, void>, art::FindOneP<T, void> >::value>::type
  check_get(FO<T, D> const & fA,
            FO<T, void> const & fAV) {
    typedef art::Ptr<typename FO<T, void>::assoc_t> item_t;
    typedef cet::maybe_ref<typename FO<T, D>::data_t const> data_t;
    check_get_one_impl<item_t, data_t>(fA, fAV);
  }

  template <typename T, typename D,
            template <typename, typename> class FM>
  typename std::enable_if<std::is_same<FM<T, void>, art::FindMany<T, void> >::value || std::is_same<FM<T, void>, art::FindManyP<T, void> >::value>::type
  check_get(FM<T, D> const & fA,
            FM<T, void> const & fAV) {
    typename FM<T, void>::value_type item;
    typename FM<T, D>::dataColl_t::value_type data;
    BOOST_CHECK_EQUAL((fAV.get(0ul, item)), 1ul);
    BOOST_CHECK_EQUAL((fAV.get(1ul, item)), 2ul);
    BOOST_CHECK_EQUAL((fAV.get(2ul, item)), 1ul);
    BOOST_CHECK_EQUAL((fA.get(0ul, item, data)), 1ul);
    BOOST_CHECK_EQUAL((fA.get(1ul, item, data)), 2ul);
    BOOST_CHECK_EQUAL((fA.get(2ul, item, data)), 1ul);
  }
}

arttest::AssnsAnalyzer::
AssnsAnalyzer(fhicl::ParameterSet const & p)
  :
  inputLabel_(p.get<std::string>("input_label")),
  testAB_(p.get<bool>("test_AB", true)),
  testBA_(p.get<bool>("test_BA", false)),
  bCollMissing_(p.get<bool>("bCollMissing", false))
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
  art::Handle<AssnsAB_t> hAB;
  art::Handle<AssnsBA_t> hBA;
  art::Handle<AssnsABV_t> hABV;
  art::Handle<AssnsBAV_t> hBAV;
  if (testAB_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hAB));
    BOOST_REQUIRE_EQUAL(hAB->size(), 3ul);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hABV));
    BOOST_REQUIRE_EQUAL(hABV->size(), 3ul);
  }
  if (testBA_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBA));
    BOOST_REQUIRE_EQUAL(hBA->size(), 3ul);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBAV));
    BOOST_REQUIRE_EQUAL(hBAV->size(), 3ul);
  }
  // Construct a FO using a handle to a collection.
  art::Handle<std::vector<size_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));

  // First, check we can make an FO on a non-existent label without
  // barfing immediately.
  FO<std::string, void> foDead(hAcoll, e, "noModule");
  BOOST_REQUIRE(!foDead.isValid());
  BOOST_REQUIRE_THROW(foDead.size(), cet::exception);

  // Now do our normal checks.
  FO<std::string, arttest::AssnTestData> foB(hAcoll, e, inputLabel_);
  FO<std::string, void> foBV(hAcoll, e, inputLabel_);
  art::Handle<std::vector<std::string> > hBcoll;
  std::unique_ptr<FO<size_t, arttest::AssnTestData> > foA;
  std::unique_ptr<FO<size_t, void> > foAV;
  if (! bCollMissing_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBcoll));
    foA.reset(new FO<size_t, arttest::AssnTestData>(hBcoll, e, inputLabel_));
    foAV.reset(new FO<size_t, void>(hBcoll, e, inputLabel_));
  }
  for (size_t i = 0; i < 3; ++i) {
    if (testAB_) {
      BOOST_CHECK_EQUAL(*(*hAB)[i].first, i);
      if (! bCollMissing_) {
        BOOST_CHECK_EQUAL(*(*hAB)[i].second, std::string(x[i]));
      }
      BOOST_CHECK_EQUAL((*hAB).data(i).d1, (*hAB)[i].first.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).d2, (*hAB)[i].second.key());
      BOOST_CHECK_EQUAL((*hAB).data(i).label, std::string(a[i]));
      BOOST_CHECK_EQUAL(*(*hABV)[i].first, i);
      if (! bCollMissing_) {
        BOOST_CHECK_EQUAL(*(*hABV)[i].second, std::string(x[i]));
      }
    }
    if (testBA_) {
      if (! bCollMissing_) {
        BOOST_CHECK_EQUAL(*(*hBA)[i].first, std::string(x[i]));
      }
      BOOST_CHECK_EQUAL(*(*hBA)[i].second, i);
      BOOST_CHECK_EQUAL((*hBA).data(i).d2, (*hBA)[i].first.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).d1, (*hBA)[i].second.key());
      BOOST_CHECK_EQUAL((*hBA).data(i).label, std::string(a[i]));
      if (! bCollMissing_) {
        BOOST_CHECK_EQUAL(*(*hBAV)[i].first, std::string(x[i]));
      }
      BOOST_CHECK_EQUAL(*(*hBAV)[i].second, i);
    }
    // Check FindOne.
    if (bCollMissing_) {
      BOOST_CHECK(!foBV.at(i));
    } else {
      BOOST_CHECK_EQUAL(dereference(foB.at(i)), std::string(x[ai[i]]));
    }
    BOOST_CHECK_EQUAL(dereference(foB.data(i)).d1, i);
    BOOST_CHECK_EQUAL(dereference(foB.data(i)).d2, bi[i]);
    if (!bCollMissing_) {
      BOOST_CHECK_EQUAL(dereference(foBV.at(i)), std::string(x[ai[i]]));
      BOOST_CHECK_EQUAL(dereference(foA->at(i)), bi[i]);
      BOOST_CHECK_EQUAL(dereference(foA->data(i)).d1, ai[i]);
      BOOST_CHECK_EQUAL(dereference(foA->data(i)).d2, i);
      BOOST_CHECK_EQUAL(dereference(foAV->at(i)), bi[i]);
      BOOST_CHECK_NO_THROW(check_get(*foA, *foAV));
    }
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
  std::unique_ptr<FO<A_t, arttest::AssnTestData> > foAv;
  if (!bCollMissing_) {
    BOOST_REQUIRE(e.getView(inputLabel_, vb));
    foAv.reset(new FO<A_t, arttest::AssnTestData>(vb, e, inputLabel_));
    BOOST_REQUIRE(*foA == *foAv);
  }
  // Check FindOne on PtrVector.
  va.vals()[1] = 0;
  art::PtrVector<A_t> pva;
  va.fill(pva);
  FO<B_t, arttest::AssnTestData> foBpv(pva, e, inputLabel_);
  BOOST_CHECK_EQUAL(foBpv.at(0), foB.at(0));
  BOOST_CHECK_EQUAL(foBpv.data(0), foB.data(0));
  BOOST_CHECK_EQUAL(foBpv.at(1), foB.at(2)); // Knocked out the middle.
  BOOST_CHECK_EQUAL(foBpv.data(1), foB.data(2));
  art::PtrVector<B_t> pvb;
  if (!bCollMissing_) {
    vb.vals()[1] = 0;
    vb.fill(pvb);
  }
  FO<A_t, arttest::AssnTestData> foApv(pvb, e, inputLabel_);
  if (! bCollMissing_) {
    BOOST_CHECK_EQUAL(foApv.at(0), foA->at(0));
    BOOST_CHECK_EQUAL(foApv.data(0), foA->data(0));
    BOOST_CHECK_EQUAL(foApv.at(1), foA->at(2)); // Knocked out the middle.
    BOOST_CHECK_EQUAL(foApv.data(1), foA->data(2));
  }
  // Check for range errors.
  BOOST_CHECK_THROW(foApv.at(3), std::out_of_range);
  BOOST_CHECK_THROW(foApv.data(3), std::out_of_range);
  // BOOST_CHECK_THROW((FO<B_t, arttest::AssnTestData> (hAcoll, e, art::InputTag(inputLabel_, "M"))), art::Exception);
}

template <template <typename, typename> class FM>
void
arttest::AssnsAnalyzer::
testMany(art::Event const & e) const
{
  art::Handle<std::vector<size_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));

  // First, check we can make an FO on a non-existent label without
  // barfing immediately.
  FM<B_t, void> fmDead(hAcoll, e, "noModule");
  BOOST_REQUIRE(!fmDead.isValid());
  BOOST_REQUIRE_THROW(fmDead.size(), cet::exception);

  // Now do our normal checks.
  // Check FindMany.
  FM<B_t, arttest::AssnTestData> fmB(hAcoll, e, art::InputTag(inputLabel_, "M"));
  BOOST_REQUIRE_EQUAL(fmB.size(), 3ul);
  if (bCollMissing_) {
    BOOST_CHECK(fmB.at(0).size() == 0);
    BOOST_CHECK(fmB.at(1).size() == 0);
    BOOST_CHECK(fmB.at(2).size() == 0);
  } else {
    BOOST_CHECK_EQUAL(fmB.at(0).size(), 1ul);
    BOOST_CHECK_EQUAL(fmB.at(1).size(), 2ul);
    BOOST_CHECK_EQUAL(fmB.at(2).size(), 1ul);
  }
  BOOST_CHECK_EQUAL(fmB.data(0).size(), 1ul);
  BOOST_CHECK_EQUAL(fmB.data(1).size(), 2ul);
  BOOST_CHECK_EQUAL(fmB.data(2).size(), 1ul);
  FM<B_t, void> fmBV(hAcoll, e, art::InputTag(inputLabel_, "M"));
  if (bCollMissing_) {
    BOOST_CHECK(fmBV.at(0).size() == 0);
    BOOST_CHECK(fmBV.at(1).size() == 0);
    BOOST_CHECK(fmBV.at(2).size() == 0);
  } else {
    BOOST_CHECK_EQUAL(fmBV.at(0).size(), 1ul);
    BOOST_CHECK_EQUAL(fmBV.at(1).size(), 2ul);
    BOOST_CHECK_EQUAL(fmBV.at(2).size(), 1ul);
    BOOST_CHECK_NO_THROW(check_get(fmB, fmBV));
  }
}

DEFINE_ART_MODULE(arttest::AssnsAnalyzer)
