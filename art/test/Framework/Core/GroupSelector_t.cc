#define BOOST_TEST_MODULE ( GroupSelector_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/test/Framework/Core/GroupSelector_t.h"

#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {

  art::BranchDescription
  makeBranchDescription(art::BranchType const bt,
                        std::string const& moduleLabel,
                        std::string const& processName,
                        std::string const& instanceName,
                        fhicl::ParameterSet const& pset,
                        art::TypeID const& producedType)
  {
    return art::BranchDescription(bt,
                                  art::TypeLabel{producedType, instanceName},
                                  art::ModuleDescription(pset.id(),
                                                         "arttest::NOMOD",
                                                         moduleLabel,
                                                         art::ProcessConfiguration(processName,
                                                                                   fhicl::ParameterSet{}.id(),
                                                                                   art::getReleaseVersion())));
  }

  void apply_gs(art::GroupSelector const& gs,
                art::ProductList const& pList,
                std::vector<bool>& results)
  {
    for (auto const& p : pList) {
      results.push_back(gs.selected(p.second));
    }
  }

  void doTest(fhicl::ParameterSet const& params,
              std::string const& testname,
              art::ProductList const& pList,
              std::vector<bool> const& expected)
  {
    std::string const parameterName{"outputCommands"};
    art::GroupSelectorRules gsr(params.get<std::vector<std::string>>(parameterName, {"keep *"}),
                                parameterName, testname);
    art::GroupSelector gs;
    gs.initialize(gsr, pList);

    std::vector<bool> results;
    apply_gs(gs, pList, results);

    BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.cbegin(), expected.cend(),
                                    results.cbegin(), results.cend());
  }

  class GlobalSetup {
  public:
    static GlobalSetup& instance() {
      static GlobalSetup s_gs;
      return s_gs;
    }

    art::ProductList& pList() { return pList_; }

  private:
    GlobalSetup(): pList_{initProductList()}
    {}

    art::ProductList pList_;

    art::ProductList initProductList()
    {
      // We pretend to have one module, with two products. The products
      // are of the same and, type differ in instance name.
      fhicl::ParameterSet modAparams;
      modAparams.put<int>("i", 2112);
      modAparams.put<std::string>("s", "hi");

      auto b1 = makeBranchDescription(art::InEvent, "modA", "PROD", "i1",
                                      modAparams,
                                      art::TypeID{typeid(arttest::ProdTypeA<std::string>)});

      auto b2 = makeBranchDescription(art::InEvent, "modA", "PROD", "i2",
                                      modAparams,
                                      art::TypeID{typeid(arttest::ProdTypeA<std::string>)});

      // Our second pretend module has only one product, and gives it no
      // instance name.
      fhicl::ParameterSet modBparams;
      modBparams.put<double>("d", 2.5);

      auto b3 = makeBranchDescription(art::InRun, "modB", "HLT", "",
                                      modBparams,
                                      art::TypeID{typeid(arttest::ProdTypeB<std::string>)});

      // Our third pretend is like modA, except it has a processName_ of
      // "USER"
      auto b4 = makeBranchDescription(art::InSubRun, "modA", "USER", "i1",
                                      modAparams,
                                      art::TypeID{typeid(arttest::ProdTypeA<std::string>)});
      auto b5 = makeBranchDescription(art::InResults, "modA", "USER", "i2",
                                      modAparams,
                                      art::TypeID{typeid(arttest::ProdTypeA<std::string>)});

      // Extra tests.
      auto b6 = makeBranchDescription(art::InRun,
                                      "ptrmvWriter",
                                      "PtrmvW",
                                      "",
                                      modAparams,
                                      art::TypeID{typeid(art::Ptr<std::string>)});

      art::ProductList const result {
        // Ordered correctly for ease of deducing expected results.
        { art::BranchKey{b6}, b6 }, // Stringart::Ptr_ptrmvWriter__PtrmvW, InRun.
        { art::BranchKey{b1}, b1 }, // ProdTypeA_modA_i1_PROD, InEvent.
        { art::BranchKey{b4}, b4 }, // ProdTypeA_modA_i1_USER, InSubRun.
        { art::BranchKey{b2}, b2 }, // ProdTypeA_modA_i2_PROD, InEvent.
        { art::BranchKey{b5}, b5 }, // ProdTypeA_modA_i2_USER, InResults.
        { art::BranchKey{b3}, b3 }  // ProdTypeB_modB__HLT, InRun.
      };
      return result;
    }
  };

  struct ProductListAccessor {
    ProductListAccessor() : pList(GlobalSetup::instance().pList()) { }
    art::ProductList& pList;
  };

}

BOOST_FIXTURE_TEST_SUITE(Tests, ProductListAccessor)

BOOST_AUTO_TEST_CASE(Test_default_parameters)
{
  std::vector<bool> const expected { true, true, true, true, true, true };
  fhicl::ParameterSet noparams;
  doTest(noparams, "default parameters", pList, expected);
}

BOOST_AUTO_TEST_CASE(Keep_all_branches_with_instance_name_i2)
{
  std::vector<bool> const expected { false, false, false, true, true, false };

  fhicl::ParameterSet keep_i2;
  std::string const keep_i2_rule = "keep *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(keep_i2_rule);
  keep_i2.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(keep_i2, "keep i2 parameters", pList, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_instance_name_i2)
{
  std::vector<bool> const expected { true, true, true, false, false, true };

  fhicl::ParameterSet drop_i2;
  std::string const drop_i2_rule1 = "keep *";
  std::string const drop_i2_rule2 = "drop *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_i2_rule1);
  cmds.push_back(drop_i2_rule2);
  drop_i2.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(drop_i2, "drop i2 parameters", pList, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_product_type_foo)
{
  std::vector<bool> const expected { true, true, true, true, true, true };

  fhicl::ParameterSet drop_foo;
  std::string const drop_foo_rule1 = "keep *_*_*_*"; // same as "keep *"
  std::string const drop_foo_rule2 = "drop foo_*_*_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_foo_rule1);
  cmds.push_back(drop_foo_rule2);
  drop_foo.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(drop_foo, "drop_foo parameters", pList, expected);
}

BOOST_AUTO_TEST_CASE(Drop_all_branches_with_product_type_ProdTypeA)
{
  std::vector<bool> const expected { true, false, false, false, false, true };

  fhicl::ParameterSet drop_ProdTypeA;
  std::string const drop_ProdTypeA_rule1 = "keep *";
  std::string const drop_ProdTypeA_rule2 = "drop *ProdTypeA_*_*_*";
  std::vector<std::string> cmds;
  cmds.push_back(drop_ProdTypeA_rule1);
  cmds.push_back(drop_ProdTypeA_rule2);
  drop_ProdTypeA.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(drop_ProdTypeA, "drop_ProdTypeA", pList, expected);
}

BOOST_AUTO_TEST_CASE(Keep_only_production_branches_with_instance_name_i1)
{
  std::vector<bool> const expected { false, true, false, false, false, false };

  fhicl::ParameterSet keep_i1prod;
  std::string const keep_i1prod_rule = "keep *_*_i1_PROD";
  std::vector<std::string> cmds;
  cmds.push_back(keep_i1prod_rule);
  keep_i1prod.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(keep_i1prod, "keep_i1prod", pList, expected);
}

BOOST_AUTO_TEST_CASE(Keep_drop_keep)
{
  std::vector<bool> const expected { true, true, true, true, true, true };

  fhicl::ParameterSet indecisive;
  std::string const indecisive_rule1 = "keep *";
  std::string const indecisive_rule2 = "drop *";
  std::string const indecisive_rule3 = "keep *";
  std::vector<std::string> cmds;
  cmds.push_back(indecisive_rule1);
  cmds.push_back(indecisive_rule2);
  cmds.push_back(indecisive_rule3);
  indecisive.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(indecisive, "indecisive", pList, expected);
}

BOOST_AUTO_TEST_CASE(Keep_all_drop_from_modA_keep_USER)
{
  std::vector<bool> const expected { true, false, true, false, true, true };

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop *_modA_*_*";
  std::string const rule3 = "keep *_*_*_USER";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  cmds.push_back(rule3);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "drop_modA_keep_user", pList, expected);
}

BOOST_AUTO_TEST_CASE(Exercise_wildcards)
{
  std::vector<bool> const expected { false, true, false, true, false, true };

  fhicl::ParameterSet params;
  std::string const rule1 = "drop *";
  std::string const rule2 = "keep *Pr*A_m?dA_??_P?O*";
  std::string const rule3 = "keep *?*?***??*????*?***_??***?__*?***T";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  cmds.push_back(rule3);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "exercise wildcards1", pList, expected);
}

BOOST_AUTO_TEST_CASE(Drop_by_full_spec)
{
  std::vector<bool> const expected { false, true, true, true, true, true };

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop Stringart::Ptr_ptrmvWriter__PtrmvW";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pList, expected);
}

BOOST_AUTO_TEST_CASE(Illegal_spec)
{
  fhicl::ParameterSet bad;
  std::string const bad_rule = "beep *_*_i2_*";
  std::vector<std::string> cmds;
  cmds.push_back(bad_rule);
  std::string const parameterName = "outputCommands";
  bad.put<std::vector<std::string> >(parameterName, cmds);
  BOOST_REQUIRE_EXCEPTION(art::GroupSelectorRules(bad.get<std::vector<std::string>>(parameterName),
                                                  parameterName, "GroupSelectorTest"),
                          art::Exception,
                          [](auto const& e) {
                            return e.categoryCode() == art::errors::Configuration;
                          });
}

BOOST_AUTO_TEST_CASE(Drop_by_type_event)
{
  std::vector<bool> const expected { true, false, true, false, true, true };

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop * InEvent";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pList, expected);
}

BOOST_AUTO_TEST_CASE(Drop_by_type_results)
{
  std::vector<bool> const expected { true, true, true, true, false, true };

  fhicl::ParameterSet params;
  std::string const rule1 = "keep *";
  std::string const rule2 = "drop * InResults";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pList, expected);
}

BOOST_AUTO_TEST_CASE(Keep_by_type_results)
{
  std::vector<bool> const expected { false, false, false, false, true, false };

  fhicl::ParameterSet params;
  std::string const rule1 = "drop *";
  std::string const rule2 = "keep *_*_*_* inResults";
  std::vector<std::string> cmds;
  cmds.push_back(rule1);
  cmds.push_back(rule2);
  params.put<std::vector<std::string> >("outputCommands", cmds);

  doTest(params, "drop product by full spec.", pList, expected);
}

BOOST_AUTO_TEST_CASE(Bad_type)
{
  fhicl::ParameterSet bad;
  std::string const bad_rule = "keep *_*_i2_* wibble";
  std::vector<std::string> cmds;
  cmds.push_back(bad_rule);
  std::string const parameterName = "outputCommands";
  bad.put<std::vector<std::string> >(parameterName, cmds);
  BOOST_REQUIRE_EXCEPTION(art::GroupSelectorRules(bad.get<std::vector<std::string>>(parameterName),
                                                  parameterName, "GroupSelectorTest"),
                          art::Exception,
                          [](auto const& e) {
                            return e.categoryCode() == art::errors::Configuration;
                          });
}

BOOST_AUTO_TEST_SUITE_END()
