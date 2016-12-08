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
}

arttest::AssnsReaderTest::
AssnsReaderTest(fhicl::ParameterSet const & ps)
  : art::EDAnalyzer(ps)
  , inputLabel_(ps.get<std::string>("inputLabel"))
  , process_(ps.get<std::string>("process", {}))
  , wantVoid_(ps.get<std::string>("wantVoid", "ALL"))
  , wantMV_(ps.get<bool>("wantMV", true))
  , wantMany_(ps.get<bool>("wantMany", true))
{
}

namespace {
  template <typename A, typename B, typename D>
  std::size_t
  expectedSize(std::string const & wantVoid [[gnu::unused]],
               bool wantMV,
               bool wantMany,
               std::string const & process [[gnu::unused]]) {
    using namespace std::string_literals;
    static constexpr bool isVoid [[gnu::unused]] = std::is_same<D, void>::value;
    std::size_t wanted = 0ull;
    wanted += 1ull;
    if (wantMany) {
      wanted += 1ull;
    }
    if (wantMV) {
      wanted += 1ull;
      if (wantMany) {
        wanted += 1ull;
      }
    }
    wanted *= 2ull; // Two writer modules.
    if (process == "ASSNSW2") {
      // FIXME: Should pull out the config for this process and
      // recursively call ourselves with the right options.
      wanted *= 2ull;
    }
    return wanted;
  }

  template <typename A, typename B, typename D>
  void
  checkExpectedSize(std::vector<art::Handle<art::Assns<A, B, D> > > const &v,
                    std::string const & wantVoid,
                    bool wantMV,
                    bool wantMany,
                    std::string const & process) {
    BOOST_CHECK_EQUAL(v.size(),
                      (expectedSize<A, B, D>(wantVoid,
                                             wantMV,
                                             wantMany,
                                             process)));
  }
}

void
arttest::AssnsReaderTest::analyze(art::Event const & e)
{
  std::size_t const vSize = (wantVoid_ == "ALL") ? 4ull : 3ull;
  std::size_t const mvVSize = (wantVoid_ != "NONE") ? 4ull : 3ull;

  auto const vSizeM = vSize + ((wantVoid_ == "ALL") ? 2ull : 1ull);
  auto const mvVSizeM = mvVSize + ((wantVoid_ != "NONE") ? 2ull : 1ull);
  // Check <A, B> and <B, A>.
  auto const hABV = e.getValidHandle<AssnsABV_t>(inputLabel_);
  BOOST_CHECK_EQUAL(hABV->size(), vSize);
  auto const hBAV = e.getValidHandle<AssnsBAV_t>(inputLabel_);
  BOOST_CHECK_EQUAL(hBAV->size(), vSize);
  if (!process_.empty()) {
    BOOST_CHECK_EQUAL(hABV.provenance()->processName(), process_);
    BOOST_CHECK_EQUAL(hBAV.provenance()->processName(), process_);
  }
  if (wantMany_) {
    auto const hABVM = e.getValidHandle<AssnsABV_t>({inputLabel_, "many"});
    BOOST_CHECK_EQUAL(hABVM->size(), vSizeM);
    auto const hBAVM = e.getValidHandle<AssnsBAV_t>({inputLabel_, "many"});
    BOOST_CHECK_EQUAL(hBAVM->size(), vSizeM);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hABVM.provenance()->processName(), process_);
      BOOST_CHECK_EQUAL(hBAVM.provenance()->processName(), process_);
    }
  }
  if (wantMV_) {
    auto const hmvABV = e.getValidHandle<AssnsABV_t>({inputLabel_, "mapvec"});
    BOOST_CHECK_EQUAL(hmvABV->size(), mvVSize);
    auto const hmvBAV = e.getValidHandle<AssnsBAV_t>({inputLabel_, "mapvec"});
    BOOST_CHECK_EQUAL(hmvBAV->size(), mvVSize);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hmvABV.provenance()->processName(), process_);
      BOOST_CHECK_EQUAL(hmvBAV.provenance()->processName(), process_);
    }
    if (wantMany_) {
      auto const hmvABVM = e.getValidHandle<AssnsABV_t>({inputLabel_, "manymapvec"});
      BOOST_CHECK_EQUAL(hmvABVM->size(), mvVSizeM);
      auto const hmvBAVM = e.getValidHandle<AssnsBAV_t>({inputLabel_, "manymapvec"});
      BOOST_CHECK_EQUAL(hmvBAVM->size(), mvVSizeM);
      if (!process_.empty()) {
        BOOST_CHECK_EQUAL(hmvABVM.provenance()->processName(), process_);
        BOOST_CHECK_EQUAL(hmvBAVM.provenance()->processName(), process_);
      }
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

  // Check expected behavior of getManyByType().
  std::vector<art::Handle<AssnsABV_t>> hSet1;
  e.getManyByType(hSet1);
  checkExpectedSize(hSet1, wantVoid_, wantMV_, wantMany_, process_);
  std::vector<art::Handle<AssnsBAV_t>> hSet2;
  e.getManyByType(hSet2);
  checkExpectedSize(hSet2, wantVoid_, wantMV_, wantMany_, process_);
  std::vector<art::Handle<AssnsAB_t>> hSet3;
  e.getManyByType(hSet3);
  checkExpectedSize(hSet3, wantVoid_, wantMV_, wantMany_, process_);
  std::vector<art::Handle<AssnsBA_t>> hSet4;
  e.getManyByType(hSet4);
  checkExpectedSize(hSet4, wantVoid_, wantMV_, wantMany_, process_);
}

DEFINE_ART_MODULE(arttest::AssnsReaderTest)
