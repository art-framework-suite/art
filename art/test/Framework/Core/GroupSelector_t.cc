#define BOOST_TEST_MODULE (GroupSelector_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/test/Framework/Core/GroupSelector_t.h"

#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {

  template <typename PROD>
  art::BranchDescription
  makeBranchDescription(art::BranchType const bt,
                        std::string const& moduleLabel,
                        std::string const& processName,
                        std::string const& instanceName,
                        fhicl::ParameterSet const& pset)
  {
    art::TypeID const producedType{typeid(PROD)};
    return art::BranchDescription{
      bt,
      art::TypeLabel{
        producedType, instanceName, art::SupportsView<PROD>::value, false},
      art::ModuleDescription{
        pset.id(),
        "arttest::NOMOD",
        moduleLabel,
        1,
        art::ProcessConfiguration{
          processName, fhicl::ParameterSet{}.id(), art::getReleaseVersion()}}};
  }

  void
  apply_gs(art::GroupSelector const& gs,
           art::ProductDescriptionsByID const& descriptions,
           std::vector<bool>& results)
  {
    for (auto const& p : descriptions) {
      results.push_back(gs.selected(p.second));
    }
  }

  void
  doTest(fhicl::ParameterSet const& params,
         std::string const& testname,
         art::ProductTables const& pTables,
         std::initializer_list<bool const> const& expected)
  {
    std::string const parameterName{"outputCommands"};
    art::GroupSelectorRules gsr(
      params.get<std::vector<std::string>>(parameterName, {"keep *"}),
      parameterName,
      testname);
    std::vector<bool> results;

    for (std::size_t i{}; i < art::NumBranchTypes; ++i) {
      auto const bt = static_cast<art::BranchType>(i);
      auto const& descriptions = pTables.descriptions(bt);
      art::GroupSelector const gs{gsr, descriptions};
      apply_gs(gs, descriptions, results);
    }

    BOOST_REQUIRE_EQUAL_COLLECTIONS(
      cbegin(expected), cend(expected), cbegin(results), cend(results));
  }

  class GlobalSetup {
  public:
    static GlobalSetup&
    instance()
    {
      static GlobalSetup s_gs;
      return s_gs;
    }

    art::ProductTables const&
    pTables()
    {
      return pTables_;
    }

  private:
    art::ProductTables const pTables_{initProductTables()};

    art::ProductTables
    initProductTables()
    {
      // We pretend to have one module, with two products. The products
      // are of the same and, type differ in instance name.
      fhicl::ParameterSet modAparams;
      modAparams.put("i", 2112);
      modAparams.put("s", "hi");

      auto b1 = makeBranchDescription<arttest::ProdTypeA<std::string>>(
        art::InEvent, "modA", "PROD", "i1", modAparams);
      auto b2 = makeBranchDescription<arttest::ProdTypeA<std::string>>(
        art::InEvent, "modA", "PROD", "i2", modAparams);

      // Our second pretend module has only one product, and gives it no
      // instance name.
      fhicl::ParameterSet modBparams;
      modBparams.put("d", 2.5);

      auto b3 = makeBranchDescription<arttest::ProdTypeB<std::string>>(
        art::InRun, "modB", "HLT", "", modBparams);

      // Our third pretend is like modA, except it has a processName_ of
      // "USER"
      auto b4 = makeBranchDescription<arttest::ProdTypeA<std::string>>(
        art::InSubRun, "modA", "USER", "i1", modAparams);
      auto b5 = makeBranchDescription<arttest::ProdTypeA<std::string>>(
        art::InResults, "modA", "USER", "i2", modAparams);

      // Extra tests.
      auto b6 = makeBranchDescription<art::Ptr<std::string>>(
        art::InRun, "ptrmvWriter", "PtrmvW", "", modAparams);

      // Ordered correctly (via BranchType, then by ProductID) for ease of
      // deducing expected results.
      art::ProductDescriptions descriptions;
      descriptions.push_back(
        std::move(b2)); // ProductID: 1458780065 |
                        // Stringarttest::ProdTypeA_modA_i2_PROD, InEvent.
      descriptions.push_back(
        std::move(b1)); // ProductID: 1729834300 |
                        // Stringarttest::ProdTypeA_modA_i1_PROD, InEvent.
      descriptions.push_back(
        std::move(b4)); // ProductID: 105444648  |
                        // Stringarttest::ProdTypeA_modA_i1_USER, InSubRun.
      descriptions.push_back(
        std::move(b6)); // ProductID: 1113343586 |
                        // Stringart::Ptr_ptrmvWriter__PtrmvW, InRun.
      descriptions.push_back(
        std::move(b3)); // ProductID: 1125702662 |
                        // Stringarttest::ProdTypeB_modB__HLT, InRun.
      descriptions.push_back(
        std::move(b5)); // ProductID: 933294005  |
                        // Stringarttest::ProdTypeA_modA_i2_USER, InResults.
      return art::ProductTables{descriptions};
    }
  };

  struct ProductListAccessor {
    art::ProductTables const& pTables{GlobalSetup::instance().pTables()};
  };

} // namespace

BOOST_FIXTURE_TEST_SUITE(Tests, ProductListAccessor)

BOOST_AUTO_TEST_CASE(Test_default_parameters)
{
  auto const expected = {true, true, true, true, true, true};
  fhicl::ParameterSet noparams;
  doTest(noparams, "default parameters", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Keep_all_branches_with_instance_name_i2)
{
  auto const expected = {true, false, false, false, false, true};

  fhicl::ParameterSet keep_i2;
  std::string const keep_i2_rule = "keep *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(keep_i2_rule);
  keep_i2.put("outputCommands", cmds);

  doTest(keep_i2, "keep i2 parameters", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_instance_name_i2)
{
  auto const expected = {false, true, true, true, true, false};

  fhicl::ParameterSet drop_i2;
  std::string const drop_i2_rule1 = "keep *";
  std::string const drop_i2_rule2 = "drop *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_i2_rule1);
  cmds.push_back(drop_i2_rule2);
  drop_i2.put("outputCommands", cmds);

  doTest(drop_i2, "drop i2 parameters", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_product_type_foo)
{
  auto const expected = {true, true, true, true, true, true};

  fhicl::ParameterSet drop_foo;
  std::string const drop_foo_rule1 = "keep *_*_*_*"; // same as "keep *"
  std::string const drop_foo_rule2 = "drop foo_*_*_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_foo_rule1);
  cmds.push_back(drop_foo_rule2);
  drop_foo.put("outputCommands", cmds);

  doTest(drop_foo, "drop_foo parameters", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_product_type_ProdTypeA)
{
  auto const expected = {false, false, false, true, true, false};

  fhicl::ParameterSet drop_ProdTypeA;
  std::string const drop_ProdTypeA_rule1 = "keep *";
  std::string const drop_ProdTypeA_rule2 = "drop *ProdTypeA_*_*_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_ProdTypeA_rule1);
  cmds.push_back(drop_ProdTypeA_rule2);
  drop_ProdTypeA.put("outputCommands", cmds);

  doTest(drop_ProdTypeA, "drop_ProdTypeA", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Keep_only_production_branches_with_instance_name_i1)
{
  auto const expected = {false, true, false, false, false, false};

  fhicl::ParameterSet keep_i1prod;
  std::string const keep_i1prod_rule = "keep *_*_i1_PROD";
  std::vector<std::string> cmds;
  cmds.push_back(keep_i1prod_rule);
  keep_i1prod.put("outputCommands", cmds);

  doTest(keep_i1prod, "keep_i1prod", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Keep_drop_keep)
{
  auto const expected = {true, true, true, true, true, true};

  fhicl::ParameterSet indecisive;
  std::string const indecisive_rule1 = "keep *";
  std::string const indecisive_rule2 = "drop *";
  std::string const indecisive_rule3 = "keep *";
  std::vector<std::string> cmds;
  cmds.push_back(indecisive_rule1);
  cmds.push_back(indecisive_rule2);
  cmds.push_back(indecisive_rule3);
  indecisive.put("outputCommands", cmds);

  doTest(indecisive, "indecisive", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Keep_all_drop_from_modA_keep_USER)
{
  auto const expected = {false, false, true, true, true, true};

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop *_modA_*_*";
  std::string const rule3 = "keep *_*_*_USER";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  cmds.push_back(rule3);
  params.put("outputCommands", cmds);

  doTest(params, "drop_modA_keep_user", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Exercise_wildcards)
{
  auto const expected = {true, true, false, false, true, false};

  fhicl::ParameterSet params;
  std::string const rule1 = "drop *";
  std::string const rule2 = "keep *Pr*A_m?dA_??_P?O*";
  std::string const rule3 = "keep *?*?***??*????*?***_??***?__*?***T";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  cmds.push_back(rule3);
  params.put("outputCommands", cmds);

  doTest(params, "exercise wildcards1", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Drop_by_full_spec)
{
  auto const expected = {true, true, true, false, true, true};

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop Stringart::Ptr_ptrmvWriter__PtrmvW";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Illegal_spec)
{
  fhicl::ParameterSet bad;
  std::string const bad_rule = "beep *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(bad_rule);
  std::string const parameterName = "outputCommands";
  bad.put(parameterName, cmds);
  BOOST_REQUIRE_EXCEPTION(
    art::GroupSelectorRules(bad.get<std::vector<std::string>>(parameterName),
                            parameterName,
                            "GroupSelectorTest"),
    art::Exception,
    [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration;
    });
}

BOOST_AUTO_TEST_CASE(Drop_by_type_event)
{
  auto const expected = {false, false, true, true, true, true};

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop * InEvent";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Drop_by_type_results)
{
  auto const expected = {true, true, true, true, true, false};

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop * InResults";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Keep_by_type_results)
{
  auto const expected = {false, false, false, false, false, true};

  fhicl::ParameterSet params;
  std::string const rule1 = "drop *";
  std::string const rule2 = "keep *_*_*_* inResults";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pTables, expected);
}

BOOST_AUTO_TEST_CASE(Bad_type)
{
  fhicl::ParameterSet bad;
  std::string const bad_rule = "keep *_*_i2_* wibble";
  std::vector<std::string> cmds;
  cmds.push_back(bad_rule);
  std::string const parameterName = "outputCommands";
  bad.put(parameterName, cmds);
  BOOST_REQUIRE_EXCEPTION(
    art::GroupSelectorRules(bad.get<std::vector<std::string>>(parameterName),
                            parameterName,
                            "GroupSelectorTest"),
    art::Exception,
    [](auto const& e) {
      return e.categoryCode() == art::errors::Configuration;
    });
}

BOOST_AUTO_TEST_SUITE_END()
