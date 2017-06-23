////////////////////////////////////////////////////////////////////////
// Class:       AssnsReaderTest
// Module Type: analyzer
// File:        AssnsReaderTest_module.cc
//
// Generated at Wed Jul 13 14:36:05 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include "cetlib/quiet_unit_test.hpp"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "art/test/TestObjects/AssnTestData.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOne.h"
#include "canvas/Persistency/Common/FindOneP.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/maybe_ref.h"

#include "boost/type_traits.hpp"

#include <iostream>
#include <type_traits>

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
  bool const wantMV_; // Expect mapvector and derived Assns.
  bool const wantMany_; // Expect many-to-many associations.
  bool const wantAmbiguous_; // Expect an extra ABD, causing ambiguity.
};

namespace {
  typedef size_t A_t;
  typedef std::string B_t;
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> AssnsABX_t;
  typedef art::Assns<std::string, size_t, arttest::AssnTestData> AssnsBAX_t;
  typedef art::Assns<size_t, std::string, std::string> AssnsABY_t;
  typedef art::Assns<std::string, size_t, std::string> AssnsBAY_t;
  typedef art::Assns<size_t, std::string> AssnsABV_t;
  typedef art::Assns<std::string, size_t> AssnsBAV_t;
}

using namespace std::string_literals;

arttest::AssnsReaderTest::
AssnsReaderTest(fhicl::ParameterSet const & ps)
  : art::EDAnalyzer(ps)
  , inputLabel_(ps.get<std::string>("inputLabel"))
  , process_(ps.get<std::string>("process", {}))
  , wantVoid_(ps.get<std::string>("wantVoid", "ALL"))
  , wantMV_(ps.get<bool>("wantMV", true))
  , wantMany_(ps.get<bool>("wantMany", true))
  , wantAmbiguous_(ps.get<bool>("wantAmbiguous", false))
{
  consumes<AssnsABV_t>(inputLabel_);
  consumes<AssnsBAV_t>(inputLabel_);
  consumes<AssnsABV_t>({inputLabel_, "many"s});
  consumes<AssnsBAV_t>({inputLabel_, "many"s});
  consumes<AssnsABV_t>({inputLabel_, "mapvec"s});
  consumes<AssnsBAV_t>({inputLabel_, "mapvec"s});
  consumes<AssnsABV_t>({inputLabel_, "manymapvec"s});
  consumes<AssnsBAV_t>({inputLabel_, "manymapvec"s});

  consumes<AssnsABX_t>(inputLabel_);
  consumes<AssnsBAX_t>(inputLabel_);
  consumes<AssnsABX_t>({inputLabel_, "many"s});
  consumes<AssnsBAX_t>({inputLabel_, "many"s});
  consumes<AssnsABX_t>({inputLabel_, "mapvec"s});
  consumes<AssnsBAX_t>({inputLabel_, "mapvec"s});
  consumes<AssnsABX_t>({inputLabel_, "manymapvec"s});
  consumes<AssnsBAX_t>({inputLabel_, "manymapvec"s});

  consumesMany<AssnsABV_t>();
  consumesMany<AssnsBAV_t>();
  consumesMany<AssnsABX_t>();
  consumesMany<AssnsBAX_t>();
}

namespace {
  template <typename A, typename B, typename D>
  std::size_t
  expectedSize(std::string const & wantVoid,
               bool wantMV,
               bool wantMany,
               bool wantAmbiguous,
               std::string const & process) {
    using namespace std::string_literals;
    static constexpr bool isVoid [[gnu::unused]] = std::is_same<D, void>::value;
    std::size_t wanted = 0ull;
    wanted += 1ull;
    if (isVoid && wantVoid != "ALL"s && wantAmbiguous) {
      // Have an Assns<A, B, Y> to deal with too.
      wanted += 1ull;
    }
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
                    bool wantAmbiguous,
                    std::string const & process) {
    BOOST_CHECK_EQUAL(v.size(),
                      (expectedSize<A, B, D>(wantVoid,
                                             wantMV,
                                             wantMany,
                                             wantAmbiguous,
                                             process)));
  }
}

void
arttest::AssnsReaderTest::analyze(art::Event const & e)
{
  std::size_t const vSize = (wantVoid_ == "ALL"s) ? 4ull : 3ull;
  std::size_t const mvVSize = (wantVoid_ != "NONE"s) ? 4ull : 3ull;

  auto const vSizeM = vSize + ((wantVoid_ == "ALL"s) ? 2ull : 1ull);
  auto const mvVSizeM = mvVSize + ((wantVoid_ != "NONE"s) ? 2ull : 1ull);

  // Check <A, B> and <B, A>.
  art::Handle<AssnsABV_t> hABV;
  try {
    e.getByLabel(inputLabel_, hABV);
    BOOST_CHECK_EQUAL(hABV->size(), vSize);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hABV.provenance()->processName(), process_);
    }
  } catch (art::Exception const & e) {
    if (!wantAmbiguous_ || wantVoid_ == "ALL") {
      throw; // Shouldn't have gotten here.
    } else { // Expected exception.
      BOOST_REQUIRE(e.categoryCode() == art::errors::ProductNotFound);
      BOOST_CHECK_EQUAL(std::string(e.what()).substr(29,69), \
                        "getByLabel: Found 2 products rather than one which match all criteria"s);
    }
  }

  art::Handle<AssnsBAV_t> hBAV;
  try {
    e.getByLabel(inputLabel_, hBAV);
    BOOST_CHECK_EQUAL(hBAV->size(), vSize);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hBAV.provenance()->processName(), process_);
    }
  } catch (art::Exception const & e) {
    if (!wantAmbiguous_ || wantVoid_ == "ALL") {
      throw; // Shouldn't have gotten here.
    } else { // Expected exception.
      BOOST_REQUIRE(e.categoryCode() == art::errors::ProductNotFound);
      BOOST_CHECK_EQUAL(std::string(e.what()).substr(29,69),             \
                        "getByLabel: Found 2 products rather than one which match all criteria"s);
    }
  }

  if (wantMany_) {
    auto const hABVM = e.getValidHandle<AssnsABV_t>({inputLabel_, "many"s});
    BOOST_CHECK_EQUAL(hABVM->size(), vSizeM);
    auto const hBAVM = e.getValidHandle<AssnsBAV_t>({inputLabel_, "many"s});
    BOOST_CHECK_EQUAL(hBAVM->size(), vSizeM);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hABVM.provenance()->processName(), process_);
      BOOST_CHECK_EQUAL(hBAVM.provenance()->processName(), process_);
    }
  }
  if (wantMV_) {
    auto const hmvABV = e.getValidHandle<AssnsABV_t>({inputLabel_, "mapvec"s});
    BOOST_CHECK_EQUAL(hmvABV->size(), mvVSize);
    auto const hmvBAV = e.getValidHandle<AssnsBAV_t>({inputLabel_, "mapvec"s});
    BOOST_CHECK_EQUAL(hmvBAV->size(), mvVSize);
    if (!process_.empty()) {
      BOOST_CHECK_EQUAL(hmvABV.provenance()->processName(), process_);
      BOOST_CHECK_EQUAL(hmvBAV.provenance()->processName(), process_);
    }
    if (wantMany_) {
      auto const hmvABVM = e.getValidHandle<AssnsABV_t>({inputLabel_, "manymapvec"s});
      BOOST_CHECK_EQUAL(hmvABVM->size(), mvVSizeM);
      auto const hmvBAVM = e.getValidHandle<AssnsBAV_t>({inputLabel_, "manymapvec"s});
      BOOST_CHECK_EQUAL(hmvBAVM->size(), mvVSizeM);
      if (!process_.empty()) {
        BOOST_CHECK_EQUAL(hmvABVM.provenance()->processName(), process_);
        BOOST_CHECK_EQUAL(hmvBAVM.provenance()->processName(), process_);
      }
    }
  }

  // Check all <A, B, V> and <B, A, V>.
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABX_t>(inputLabel_)->size(), 3ull);
  BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAX_t>(inputLabel_)->size(), 3ull);
  if (wantMany_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABX_t>({inputLabel_, "many"s})->size(), 4ull);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAX_t>({inputLabel_, "many"s})->size(), 4ull);
  }
  if (wantMV_) {
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABX_t>({inputLabel_, "mapvec"s})->size(), 3ull);
    BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAX_t>({inputLabel_, "mapvec"s})->size(), 3ull);
    if (wantMany_) {
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsABX_t>({inputLabel_, "manymapvec"s})->size(), 4ull);
      BOOST_CHECK_EQUAL(e.getValidHandle<AssnsBAX_t>({inputLabel_, "manymapvec"s})->size(), 4ull);
    }
  }

  // Check expected behavior of getManyByType().
  std::vector<art::Handle<AssnsABV_t>> hSet1;
  e.getManyByType(hSet1);
  checkExpectedSize(hSet1, wantVoid_, wantMV_, wantMany_, wantAmbiguous_, process_);
  std::vector<art::Handle<AssnsBAV_t>> hSet2;
  e.getManyByType(hSet2);
  checkExpectedSize(hSet2, wantVoid_, wantMV_, wantMany_, wantAmbiguous_, process_);
  std::vector<art::Handle<AssnsABX_t>> hSet3;
  e.getManyByType(hSet3);
  checkExpectedSize(hSet3, wantVoid_, wantMV_, wantMany_, wantAmbiguous_, process_);
  std::vector<art::Handle<AssnsBAX_t>> hSet4;
  e.getManyByType(hSet4);
  checkExpectedSize(hSet4, wantVoid_, wantMV_, wantMany_, wantAmbiguous_, process_);
}

DEFINE_ART_MODULE(arttest::AssnsReaderTest)
