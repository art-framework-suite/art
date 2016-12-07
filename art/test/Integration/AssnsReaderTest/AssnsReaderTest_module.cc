////////////////////////////////////////////////////////////////////////
// Class:       AssnsReaderTest
// Module Type: analyzer
// File:        AssnsReaderTest_module.cc
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
   class AssnsReaderTest;
}

#include <initializer_list>

class arttest::AssnsReaderTest : public art::EDAnalyzer {
public:
  explicit AssnsReaderTest(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;

private:

  std::string const inputLabel_;
  std::string const process_;
  std::string const wantVoid_; // "ALL," "NONE," or "SOME."
  bool const wantMV_; // Produce mapvector and derived Assns.
  bool const wantMany_; // Produce many-to-many associations.
};

namespace {
  typedef size_t A_t;
  typedef std::string B_t;
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> AssnsAB_t;
  typedef art::Assns<std::string, size_t, arttest::AssnTestData> AssnsBA_t;
  typedef art::Assns<size_t, std::string> AssnsABV_t;
  typedef art::Assns<std::string, size_t> AssnsBAV_t;

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

}

arttest::AssnsReaderTest::
AssnsReaderTest(fhicl::ParameterSet const & ps)
  : art::EDAnalyzer(ps)
  , inputLabel_(ps.get<std::string>("inputLabel"))
  , process_(ps.get<std::string>("process"))
  , wantVoid_(ps.get<std::string>("wantVoid", "ALL"))
  , wantMV_(ps.get<bool>("wantMV", true))
  , wantMany_(ps.get<bool>("wantMany", true))
{
}

void
arttest::AssnsReaderTest::analyze(art::Event const & e)
{
  std::cerr << "wantVoid_ = " << wantVoid_ << ".\n";
  std::size_t const vSize = (wantVoid_ == "ALL") ? 4ull : 3ull;
  std::size_t const mvVSize = (wantVoid_ != "NONE") ? 4ull : 3ull;

  auto const vSizeM = vSize + ((wantVoid_ == "ALL") ? 2ull : 1ull);
  auto const mvVSizeM = mvVSize + ((wantVoid_ != "NONE") ? 2ull : 1ull);
  // Check <A, B> and <B, A>.
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABV_t>(inputLabel_)->size(), vSize);
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAV_t>(inputLabel_)->size(), vSize);
  if (wantMany_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABV_t>({inputLabel_, "many"})->size(), vSizeM);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAV_t>({inputLabel_, "many"})->size(), vSizeM);
  }
  if (wantMV_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABV_t>({inputLabel_, "mapvec"})->size(), mvVSize);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAV_t>({inputLabel_, "mapvec"})->size(), mvVSize);
    if (wantMany_) {
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABV_t>({inputLabel_, "manymapvec"})->size(), mvVSizeM);
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAV_t>({inputLabel_, "manymapvec"})->size(), mvVSizeM);
    }
  }
  

  // Check all <A, B, V> and <B, A, V>.
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsAB_t>(inputLabel_)->size(), 3ull);
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBA_t>(inputLabel_)->size(), 3ull);
  if (wantMany_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsAB_t>({inputLabel_, "many"})->size(), 4ull);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBA_t>({inputLabel_, "many"})->size(), 4ull);
  }
  if (wantMV_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsAB_t>({inputLabel_, "mapvec"})->size(), 3ull);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBA_t>({inputLabel_, "mapvec"})->size(), 3ull);
    if (wantMany_) {
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsAB_t>({inputLabel_, "manymapvec"})->size(), 4ull);
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBA_t>({inputLabel_, "manymapvec"})->size(), 4ull);
    }
  }

}

DEFINE_ART_MODULE(arttest::AssnsReaderTest)
