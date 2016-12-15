////////////////////////////////////////////////////////////////////////
// Class:       AssnsAnalyzer
// Module Type: analyzer
// File:        AssnsAnalyzer_module.cc
//
// Generated at Wed Jul 13 14:36:05 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOne.h"
#include "canvas/Persistency/Common/FindOneP.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/maybe_ref.h"
#include "art/test/TestObjects/AssnTestData.h"

#include "cetlib/quiet_unit_test.hpp"
#include "boost/type_traits.hpp"

#include <type_traits>

#include <iostream>

namespace arttest {
   class AssnsAnalyzer;
}

#include <initializer_list>

class arttest::AssnsAnalyzer : public art::EDAnalyzer {
public:
  explicit AssnsAnalyzer(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;

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
  typedef art::Assns<A_t, B_t, arttest::AssnTestData> AssnsAB_t;
  typedef art::Assns<B_t, size_t, arttest::AssnTestData> AssnsBA_t;
  typedef art::Assns<A_t, B_t> AssnsABV_t;
  typedef art::Assns<B_t, size_t> AssnsBAV_t;

  // function template to allow us to dereference both maybe_ref<T>
  // objects and objects that have an operator*.
  template <class R, class W> R const& dereference(W const& wrapper);

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

  // These are the expected answers; they are global data used in
  // check_get_one_impl.
  char const* const X[] = { "zero", "one", "two" };
  char const* const A[] = { "A", "B", "C" };
  size_t const AI[] = { 2, 0, 1 }; // Order in Acoll
  size_t const BI[] = { 1, 2, 0 }; // Order in Bcoll

  // check_get_one_impl is the common implementation for the check_get
  // functions.
  template <typename I, typename D, typename F, typename FV>
  void
  check_get_one_impl(F const & fA, FV const & fAV) {
    I item;
    D data;
    for (size_t i = 0; i < 3; ++i) {
      fA.get(i, item, data);
      BOOST_CHECK_EQUAL(dereference(item), BI[i]);
      BOOST_CHECK_EQUAL(dereference(data).d1, AI[i]);
      BOOST_CHECK_EQUAL(dereference(data).d2, i);
      fAV.get(i, item);
      BOOST_CHECK_EQUAL(dereference(item), BI[i]);
    }
  }

  // check_get tests that both fA and fAV both contain the
  // expected information, as recorded in the global variables.
  // Each looks like:
  //    template <typename T, typename D, template <typename, typename> class F>
  //    void
  //    check_get(F<T,D> const&, F<T,void> const&);
  // with complications using enable_if to control which overload is
  // called in a given case.

  // check_get specialized for FindOne
  template <typename T, typename D,
            template <typename, typename> class FO>
  typename std::enable_if<std::is_same<FO<T, void>, art::FindOne<T, void> >::value>::type
  check_get(FO<T, D> const & fA,
            FO<T, void> const & fAV) {
    typedef cet::maybe_ref<typename FO<T, void>::assoc_t const> item_t;
    typedef cet::maybe_ref<typename FO<T, D>::data_t const> data_t;
    check_get_one_impl<item_t, data_t>(fA, fAV);
  }

  // check_get specialized for FindOneP
  template <typename T, typename D,
            template <typename, typename> class FO>
  //typename std::enable_if<std::is_same<FO<T, void>, art::FindOneP<T, void> >::value>::type
  typename std::enable_if<std::is_same<FO<T, void>, art::FindOneP<T, void> >::value>::type
  check_get(FO<T, D> const & fA,
            FO<T, void> const & fAV) {
    typedef art::Ptr<typename FO<T, void>::assoc_t> item_t;
    typedef cet::maybe_ref<typename FO<T, D>::data_t const> data_t;
    check_get_one_impl<item_t, data_t>(fA, fAV);
  }

  // check_get specialized for FindMany and FindManyP
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
  art::EDAnalyzer(p),
  inputLabel_(p.get<std::string>("input_label")),
  testAB_(p.get<bool>("test_AB", true)),
  testBA_(p.get<bool>("test_BA", false)),
  bCollMissing_(p.get<bool>("bCollMissing", false))
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

template <template <typename, typename = void> class FO>
void
arttest::AssnsAnalyzer::
testOne(art::Event const & e) const
{
  // Behavior on missing bColl is different for FindOne vs FindOneP.
  static constexpr
    bool isFOP =
    std::is_same<typename FO<B_t>::value_type, art::Ptr<typename FO<B_t>::assoc_t> >::value;
  bool const extendedTestsOK = isFOP || (!bCollMissing_);
  art::Handle<AssnsAB_t> hAB;
  art::Handle<AssnsBA_t> hBA;
  art::Handle<AssnsABV_t> hABV;
  art::Handle<AssnsBAV_t> hBAV;
  if (testAB_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hAB));
    BOOST_REQUIRE_EQUAL(hAB->size(), 3ul);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hABV));
    BOOST_REQUIRE_EQUAL(hABV->size(), 3ul);
    // Check alternative accessors and range checking for Assns.
    BOOST_CHECK_THROW((*hAB).at(3), std::out_of_range);
    BOOST_CHECK_EQUAL(&(*hAB).data(0), &(*hAB).data((*hAB).begin()));
    BOOST_CHECK_THROW((*hAB).data(3), std::out_of_range);
  }
  if (testBA_) {
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBA));
    BOOST_REQUIRE_EQUAL(hBA->size(), 3ul);
    BOOST_REQUIRE(e.getByLabel(inputLabel_, hBAV));
    BOOST_REQUIRE_EQUAL(hBAV->size(), 3ul);
    // Check alternative accessors and range checking for Assns.
    BOOST_CHECK_THROW((*hBA).at(3), std::out_of_range);
    BOOST_CHECK_EQUAL(&(*hBA).data(0), &(*hBA).data((*hBA).begin()));
    BOOST_CHECK_THROW((*hBA).data(3), std::out_of_range);
  }
  // Construct a FO using a handle to a collection.
  art::Handle<std::vector<A_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));
  auto vhAcoll = e.getValidHandle<std::vector<A_t> >(inputLabel_);
  // First, check we can make an FO on a non-existent label without
  // barfing immediately.
  FO<B_t, arttest::AssnTestData> foDead(hAcoll, e, "noModule");
  BOOST_REQUIRE(!foDead.isValid());
  BOOST_REQUIRE_EXCEPTION(foDead.size(),                                \
                          art::Exception,                               \
                          [](art::Exception const & e)                  \
                          {                                             \
                            return e.categoryCode() == art::errors::LogicError && \
                              e.history().back() == "ProductNotFound";  \
                          });                                           \
  BOOST_REQUIRE_EXCEPTION(foDead.data(0),                               \
                          art::Exception,                               \
                          [](art::Exception const & e)                  \
                          {                                             \
                            return e.categoryCode() == art::errors::LogicError && \
                              e.history().back() == "ProductNotFound";  \
                          });

  // Now do our normal checks.
  art::Handle<std::vector<B_t> > hBcoll;
  BOOST_REQUIRE_EQUAL(e.getByLabel(inputLabel_, hBcoll), !bCollMissing_);
  std::unique_ptr<FO<A_t, arttest::AssnTestData> > foA;
  std::unique_ptr<FO<A_t> > foAV;
  if (! bCollMissing_) {
    foA.reset(new FO<A_t, arttest::AssnTestData>(hBcoll, e, inputLabel_));
    foAV.reset(new FO<A_t>(hBcoll, e, inputLabel_));
  }
  if (extendedTestsOK) {
    FO<B_t, arttest::AssnTestData> foB(hAcoll, e, inputLabel_);
    FO<B_t, arttest::AssnTestData> foB2(vhAcoll, e, inputLabel_);
    FO<B_t> foBV(hAcoll, e, inputLabel_);
    std::vector<art::Ptr<A_t> > vp;
    vp.reserve(3);
    for (size_t i = 0; i < 3; ++i) {
      vp.emplace_back(hAcoll, i);
      if (testAB_) {
        BOOST_CHECK_EQUAL(*(*hAB)[i].first, i);
        if (! bCollMissing_) {
          BOOST_CHECK_EQUAL(*(*hAB)[i].second, std::string(X[i]));
        }
        BOOST_CHECK_EQUAL((*hAB).data(i).d1, (*hAB)[i].first.key());
        BOOST_CHECK_EQUAL((*hAB).data(i).d2, (*hAB)[i].second.key());
        BOOST_CHECK_EQUAL((*hAB).data(i).label, std::string(A[i]));
        BOOST_CHECK_EQUAL(*(*hABV)[i].first, i);
        if (! bCollMissing_) {
          BOOST_CHECK_EQUAL(*(*hABV)[i].second, std::string(X[i]));
        }
      }
      if (testBA_) {
        if (! bCollMissing_) {
          BOOST_CHECK_EQUAL(*(*hBA)[i].first, std::string(X[i]));
        }
        BOOST_CHECK_EQUAL(*(*hBA)[i].second, i);
        BOOST_CHECK_EQUAL((*hBA).data(i).d2, (*hBA)[i].first.key());
        BOOST_CHECK_EQUAL((*hBA).data(i).d1, (*hBA)[i].second.key());
        BOOST_CHECK_EQUAL((*hBA).data(i).label, std::string(A[i]));
        if (! bCollMissing_) {
          BOOST_CHECK_EQUAL(*(*hBAV)[i].first, std::string(X[i]));
        }
        BOOST_CHECK_EQUAL(*(*hBAV)[i].second, i);
      }
      if (bCollMissing_) {
        BOOST_CHECK(!foBV.at(i));
      }
      else {
        BOOST_CHECK_EQUAL(dereference(foBV.at(i)), std::string(X[AI[i]]));
        BOOST_CHECK_EQUAL(dereference(foA->at(i)), BI[i]);
        BOOST_CHECK_EQUAL(dereference(foA->data(i)).d1, AI[i]);
        BOOST_CHECK_EQUAL(dereference(foA->data(i)).d2, i);
        BOOST_CHECK_EQUAL(dereference(foAV->at(i)), BI[i]);
        BOOST_CHECK_NO_THROW(check_get(*foA, *foAV));
      }
      for (auto const & f : { foB, foB2 } ) {
        if (!bCollMissing_) {
          BOOST_CHECK_EQUAL(dereference(f.at(i)), std::string(X[AI[i]]));
        }
        BOOST_CHECK_EQUAL(dereference(f.data(i)).d1, i);
        BOOST_CHECK_EQUAL(dereference(f.data(i)).d2, BI[i]);
      }
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
    // Check FindOne on shuffled reference collection.
    auto va2 = va.vals(); // Copy.
    {
      using std::swap;
      swap(*va2.begin(), *(va2.begin()+1));
      swap(*va2.begin(), *(va2.begin()+2));
    }
    BOOST_REQUIRE(va.vals() != va2);
    FO<B_t, arttest::AssnTestData> foBv2(va2, e, inputLabel_);
    for (size_t i = 0, e = foBv2.size(); i != e; ++i) {
      using std::find;
      auto it = find(va.begin(), va.end(), va2[i]);
      BOOST_REQUIRE(it != va.end());
      BOOST_REQUIRE(foBv.at(std::distance(va.begin(), it)) == foBv2.at(i));
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
    // Check for range errors.
    BOOST_CHECK_THROW(foApv.at(3), std::out_of_range);
    BOOST_CHECK_THROW(foApv.data(3), std::out_of_range);

    if (! bCollMissing_) {
      BOOST_CHECK_EQUAL(foApv.at(0), foA->at(0));
      BOOST_CHECK_EQUAL(foApv.data(0), foA->data(0));
      BOOST_CHECK_EQUAL(foApv.at(1), foA->at(2)); // Knocked out the middle.
      BOOST_CHECK_EQUAL(foApv.data(1), foA->data(2));
    }

    // Check FindOne looking into a map_vector.
    BOOST_REQUIRE(hAcoll.isValid());
    art::InputTag tag(inputLabel_, "mapvec");
    FO<B_t, arttest::AssnTestData> foBmv(hAcoll, e, tag);
    if (! bCollMissing_) {
      BOOST_CHECK_EQUAL(dereference(foBmv.at(0)), dereference(foB.at(0)));
      BOOST_CHECK_EQUAL(dereference(foBmv.data(0)).label, dereference(foB.data(0)).label);
      BOOST_CHECK_EQUAL(dereference(foBmv.at(1)), dereference(foB.at(1)));
      BOOST_CHECK_EQUAL(dereference(foBmv.data(1)).label, dereference(foB.data(1)).label);
      BOOST_CHECK_EQUAL(dereference(foBmv.at(2)), dereference(foB.at(2)));
      BOOST_CHECK_EQUAL(dereference(foBmv.data(2)).label, dereference(foB.data(2)).label);
    }
    // ... and a View into a map_vector.
    art::View<B_t> vmvb;
    if (!bCollMissing_) {
      BOOST_REQUIRE(e.getView(inputLabel_, "mv", vmvb));
      FO<A_t, arttest::AssnTestData> foAmvv(vmvb, e, tag);
      for (std::size_t i = 0ul, sz = foAmvv.size(); i != sz; ++i) {
        BOOST_REQUIRE_EQUAL(dereference(foAmvv.at(i)), i);
      }
    }
  }
}

template <template <typename, typename = void> class FM>
void
arttest::AssnsAnalyzer::
testMany(art::Event const & e) const
{
  static constexpr
    bool isFMP [[gnu::unused]] =
    std::is_same<typename FM<B_t>::value_type, art::Ptr<typename FM<B_t>::assoc_t> >::value;
  art::Handle<std::vector<A_t> > hAcoll;
  BOOST_REQUIRE(e.getByLabel(inputLabel_, hAcoll));

  // First, check we can make an FM on a non-existent label without
  // barfing immediately.
  FM<B_t, arttest::AssnTestData> fmDead(hAcoll, e, "noModule");
  BOOST_REQUIRE(!fmDead.isValid());
  BOOST_REQUIRE_EXCEPTION(fmDead.size(),                                \
                          art::Exception,                               \
                          [](art::Exception const & e)                  \
                          {                                             \
                            return e.categoryCode() == art::errors::LogicError && \
                              e.history().back() == "ProductNotFound";  \
                          });                                           \
  BOOST_REQUIRE_EXCEPTION(fmDead.data(0),                               \
                          art::Exception,                               \
                          [](art::Exception const & e)                  \
                          {                                             \
                            return e.categoryCode() == art::errors::LogicError && \
                              e.history().back() == "ProductNotFound";  \
                          });

  // Now do our normal checks.
  // Check FindMany.
  FM<B_t, arttest::AssnTestData>
    fmB(hAcoll, e, art::InputTag(inputLabel_, "many"));
  art::Ptr<A_t> larry(hAcoll, 0), curly(hAcoll, 1), mo(hAcoll, 2);
  FM<B_t, arttest::AssnTestData>
    fmB2({larry, curly, mo}, e, art::InputTag(inputLabel_, "many"));
  for (auto const & f : { fmB, fmB2 }) {
    BOOST_REQUIRE_EQUAL(f.size(), 3ul);
    BOOST_CHECK_EQUAL(f.at(0).size(), 1ul);
    BOOST_CHECK_EQUAL(f.at(1).size(), 2ul);
    BOOST_CHECK_EQUAL(f.at(2).size(), 1ul);
    BOOST_CHECK_EQUAL(f.data(0).size(), 1ul);
    BOOST_CHECK_EQUAL(f.data(1).size(), 2ul);
    BOOST_CHECK_EQUAL(f.data(2).size(), 1ul);
  }
  FM<B_t> fmBV(hAcoll, e, art::InputTag(inputLabel_, "many"));
  BOOST_CHECK_EQUAL(fmBV.at(0).size(), 1ul);
  BOOST_CHECK_EQUAL(fmBV.at(1).size(), 2ul);
  BOOST_CHECK_EQUAL(fmBV.at(2).size(), 1ul);
  BOOST_CHECK_NO_THROW(check_get(fmB, fmBV));
  // Check FindMany on View.
  art::View<A_t> va;
  BOOST_REQUIRE(e.getView(inputLabel_, va));
  FM<B_t> fmBv(va, e, art::InputTag(inputLabel_, "many"));
  BOOST_REQUIRE(fmBV == fmBv);
  // Check FindMany on shuffled reference collection.
  auto va2 = va.vals(); // Copy.
  {
    using std::swap;
    swap(*va2.begin(), *(va2.begin()+1));
    swap(*va2.begin(), *(va2.begin()+2));
  }
  BOOST_REQUIRE(va.vals() != va2);
  FM<B_t> fmBv2(va2, e, art::InputTag(inputLabel_, "many"));
  for (size_t i = 0, e = fmBv2.size(); i != e; ++i) {
    using std::find;
    auto it = find(va.begin(), va.end(), va2[i]);
    BOOST_REQUIRE(it != va.end());
    BOOST_REQUIRE(fmBv.at(std::distance(va.begin(), it)) == fmBv2.at(i));
  }

  // Check FindMany on map_vector
  FM<B_t> fmvBV(hAcoll, e, art::InputTag(inputLabel_, "manymapvec"));
  if (!bCollMissing_) {
    BOOST_CHECK_EQUAL(fmvBV.at(0).size(), 1ul);
    BOOST_CHECK_EQUAL(fmvBV.at(1).size(), 2ul);
    BOOST_CHECK_EQUAL(fmvBV.at(2).size(), 1ul);
    BOOST_CHECK_NO_THROW(check_get(fmB, fmBV));
  }
}

DEFINE_ART_MODULE(arttest::AssnsAnalyzer)
