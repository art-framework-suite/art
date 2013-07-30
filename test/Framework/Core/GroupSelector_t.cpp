#include "GroupSelector_t.h"

#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/Exception.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {
  art::BranchKey
  make_BranchKey(art::BranchDescription const &b) {
    return art::BranchKey(b.friendlyClassName(),
                          b.moduleLabel(),
                          b.productInstanceName(),
                          b.processName());
  }

  art::BranchDescription
  makeBranchDescription(art::BranchType bt,
                        std::string const &moduleLabel,
                        std::string const &processName,
                        std::string const &instanceName,
                        fhicl::ParameterSet const &pset,
                        art::TypeID const &producedType) {
    return
      art::BranchDescription(art::TypeLabel(bt, producedType, instanceName),
                             art::ModuleDescription
                             (pset.id(),
                              "arttest::NOMOD",
                              moduleLabel,
                              art::ProcessConfiguration(processName,
                                                        fhicl::ParameterSet().id(),
                                                        art::getReleaseVersion(),
                                                        "")));
  }

  void apply_gs(art::GroupSelector const& gs,
                art::ProductList const &pList,
                std::vector<bool>& results)
  {
    for (art::ProductList::const_iterator
           it = pList.begin(),
           end = pList.end();
         it != end;
         ++it) {
      results.push_back(gs.selected(it->second));
    }
  }
}

int doTest(fhicl::ParameterSet const& params,
           char const* testname,
           art::ProductList const &pList,
           std::vector<bool>& expected)
{
  art::GroupSelectorRules gsr(params, "outputCommands", testname);
  art::GroupSelector gs;
  gs.initialize(gsr, pList);
  std::cout << "GroupSelector from "
            << testname
            << ": "
            << gs
            << std::endl;

  std::vector<bool> results;
  apply_gs(gs, pList, results);

  int rc = 0;
  if (expected != results) rc = 1;
  if (rc == 1) std::cout << "FAILURE: " << testname << '\n';
  std::cout << "---" << std::endl;
  return rc;
}

int work()
{
  int rc = 0;
  // We pretend to have one module, with two products. The products
  // are of the same and, type differ in instance name.
  fhicl::ParameterSet modAparams;
  modAparams.put<int>("i", 2112);
  modAparams.put<std::string>("s", "hi");

  art::BranchDescription
    b1(makeBranchDescription(art::InEvent, "modA", "PROD", "i1",
                             modAparams,
                             art::TypeID(typeid(arttest::ProdTypeA<std::string>))));
  art::BranchDescription
    b2(makeBranchDescription(art::InEvent, "modA", "PROD", "i2",
                             modAparams,
                             art::TypeID(typeid(arttest::ProdTypeA<std::string>))));

  // Our second pretend module has only one product, and gives it no
  // instance name.
  fhicl::ParameterSet modBparams;
  modBparams.put<double>("d", 2.5);

  art::BranchDescription
    b3(makeBranchDescription(art::InEvent, "modB", "HLT", "",
                             modBparams,
                             art::TypeID(typeid(arttest::ProdTypeB<std::string>))));

  // Our third pretend is like modA, except it hass processName_ of
  // "USER"
  art::BranchDescription
    b4(makeBranchDescription(art::InEvent, "modA", "USER", "i1",
                             modAparams,
                             art::TypeID(typeid(arttest::ProdTypeA<std::string>))));
  art::BranchDescription
    b5(makeBranchDescription(art::InEvent, "modA", "USER", "i2",
                             modAparams,
                             art::TypeID(typeid(arttest::ProdTypeA<std::string>))));

  // Extra tests.
  art::BranchDescription
    b6(makeBranchDescription(art::InEvent,
                             "ptrmvWriter",
                             "PtrmvW",
                             "",
                             modAparams,
                             art::TypeID(typeid(art::Ptr<std::string>))));

  // These are pointers to all the branches that are available. In a
  // framework program, these would come from the MasterProductRegistry
  // which is used to initialze the OutputModule being configured.
  art::ProductList pList;
  pList.insert(std::make_pair(make_BranchKey(b1), b1)); // ProdTypeA_modA_i1_PROD
  pList.insert(std::make_pair(make_BranchKey(b2), b2)); // ProdTypeA_modA_i2_PROD
  pList.insert(std::make_pair(make_BranchKey(b3), b3)); // ProdTypeB_modB__HLT.
  pList.insert(std::make_pair(make_BranchKey(b4), b4)); // ProdTypeA_modA_i1_USER.
  pList.insert(std::make_pair(make_BranchKey(b5), b5)); // ProdTypeA_modA_i2_USER.
  pList.insert(std::make_pair(make_BranchKey(b6), b6)); // Stringart::Ptr_ptrmvWriter__PtrmvW

  // Test default parameters
  {
    bool wanted[] = { true, true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));
    fhicl::ParameterSet noparams;

    rc += doTest(noparams, "default parameters", pList, expected);
  }

  // Keep all branches with instance name i2.
  {
    bool wanted[] = { false, false, false, true, true, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet keep_i2;
    std::string const keep_i2_rule = "keep *_*_i2_*";
    std::vector<std::string> cmds;
    cmds.push_back(keep_i2_rule);
    keep_i2.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(keep_i2, "keep_i2 parameters", pList, expected);
  }

  // Drop all branches with instance name i2.
  {
    bool wanted[] = { true, true, true, false, false, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_i2;
    std::string const drop_i2_rule1 = "keep *";
    std::string const drop_i2_rule2 = "drop *_*_i2_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_i2_rule1);
    cmds.push_back(drop_i2_rule2);
    drop_i2.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_i2, "drop_i2 parameters", pList, expected);
  }

  // Now try dropping all branches with product type "foo". There are
  // none, so all branches should be written.
  {
    bool wanted[] = { true, true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_foo;
    std::string const drop_foo_rule1 = "keep *_*_*_*"; // same as "keep *"
    std::string const drop_foo_rule2 = "drop foo_*_*_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_foo_rule1);
    cmds.push_back(drop_foo_rule2);
    drop_foo.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_foo, "drop_foo parameters", pList, expected);
  }

  // Now try dropping all branches with product type "ProdTypeA".
  {
    bool wanted[] = { true, false, false, false, false, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_ProdTypeA;
    std::string const drop_ProdTypeA_rule1 = "keep *";
    std::string const drop_ProdTypeA_rule2 = "drop *ProdTypeA_*_*_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_ProdTypeA_rule1);
    cmds.push_back(drop_ProdTypeA_rule2);
    drop_ProdTypeA.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_ProdTypeA,
                 "drop_ProdTypeA",
                 pList, expected);
  }

  // Keep only branches with instance name 'i1', from Production.
  {
    bool wanted[] = { false, true, false, false, false, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet keep_i1prod;
    std::string const keep_i1prod_rule = "keep *_*_i1_PROD";
    std::vector<std::string> cmds;
    cmds.push_back(keep_i1prod_rule);
    keep_i1prod.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(keep_i1prod,
                 "keep_i1prod",
                 pList, expected);
  }

  // First say to keep everything,  then  to drop everything, then  to
  // keep it again. The end result should be to keep everything.
  {
    bool wanted[] = { true, true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet indecisive;
    std::string const indecisive_rule1 = "keep *";
    std::string const indecisive_rule2 = "drop *";
    std::string const indecisive_rule3 = "keep *";
    std::vector<std::string> cmds;
    cmds.push_back(indecisive_rule1);
    cmds.push_back(indecisive_rule2);
    cmds.push_back(indecisive_rule3);
    indecisive.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(indecisive,
                 "indecisive",
                 pList, expected);
  }

  // Keep all things, but drop all things from modA, but later keep all
  // things from USER.
  {
    bool wanted[] = { true, false, true, false, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet params;
    std::string const rule1 = "keep *";
    std::string const rule2 = "drop *_modA_*_*";
    std::string const rule3 = "keep *_*_*_USER";
    std::vector<std::string> cmds;
    cmds.push_back(rule1);
    cmds.push_back(rule2);
    cmds.push_back(rule3);
    params.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(params,
                 "drop_modA_keep_user",
                 pList, expected);
  }

  // Exercise the wildcards * and ?
  {
    bool wanted[] = { false, true, false, true, false, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet params;
    std::string const rule1 = "drop *";
    std::string const rule2 = "keep *Pr*A_m?dA_??_P?O*";
    std::string const rule3 = "keep *?*?***??*????*?***_??***?__*?***T";
    std::vector<std::string> cmds;
    cmds.push_back(rule1);
    cmds.push_back(rule2);
    cmds.push_back(rule3);
    params.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(params,
                 "exercise wildcards1",
                 pList, expected);
  }

  // Drop one product specifically by full specification.
  {
    bool wanted[] = { false, true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet params;
    std::string const rule1 = "keep *";
    std::string const rule2 = "drop Stringart::Ptr_ptrmvWriter__PtrmvW";
    std::vector<std::string> cmds;
    cmds.push_back(rule1);
    cmds.push_back(rule2);
    params.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(params,
                 "drop product by full spec.",
                 pList, expected);
  }

  {
    // Now try an illegal specification: not starting with 'keep' or 'drop'
    try {
        fhicl::ParameterSet bad;
        std::string const bad_rule = "beep *_*_i2_*";
        std::vector<std::string> cmds;
        cmds.push_back(bad_rule);
        bad.put<std::vector<std::string> >("outputCommands", cmds);
        art::GroupSelectorRules gsr(bad, "outputCommands", "GroupSelectorTest");
        art::GroupSelector gs;
        gs.initialize(gsr, pList);
        std::cerr << "Failed to throw required exception\n";
        rc += 1;
    }
    catch (art::Exception const& x) {
        // OK, we should be here... now check exception type
        assert (x.categoryCode() == art::errors::Configuration);
    }
    catch (...) {
        std::cerr << "Wrong exception type\n";
        rc += 1;
    }
  }

  return rc;
}

int main()
{
  art::RootDictionaryManager rdm;
  int rc = 0;
  try
    {
      rc = work();
    }
  catch (art::Exception& x)
    {
      std::cerr << "art::Exception caught:\n" << x << '\n';
      rc = 1;
    }
  catch (...)
    {
      std::cerr << "Unknown exception caught\n";
      rc = 2;
    }
  return rc;
}


