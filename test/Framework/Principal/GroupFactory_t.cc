// Test of GroupFactory functions

#define BOOST_TEST_MODULE (GroupFactory_t)
#include "boost/test/auto_unit_test.hpp"

#include <iostream>
#include <typeinfo>
#include <vector>

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Utilities/WrappedClassName.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace {
  art::BranchDescription
  makeBranchDescription(art::BranchType bt,
                        std::string const &moduleLabel,
                        std::string const &processName,
                        std::string const &instanceName,
                        fhicl::ParameterSet const &pset,
                        art::TypeID const &producedType) {
    return
      art::BranchDescription(art::TypeLabel(bt, producedType, instanceName),
                             art::ModuleDescription(pset.id(),
                                                    "arttest::NOMOD",
                                                    moduleLabel,
                                                    art::ProcessConfiguration
                                                    (processName,
                                                     fhicl::ParameterSet().id(),
                                                     art::getReleaseVersion(),
                                                     "")));
  }
}

struct TestFixture {
  TestFixture() {
//    static art::RootDictionaryManager rdm_s;
  }
};

BOOST_FIXTURE_TEST_SUITE(GroupFactory_t, TestFixture)

BOOST_AUTO_TEST_CASE(Group)
{
  std::unique_ptr<art::Group>
    g(art::gfactory::make_group(makeBranchDescription(art::InEvent,
                                                      "GGen",
                                                      "GEN",
                                                      "G1",
                                                      fhicl::ParameterSet(),
                                                      art::TypeID(typeid(int))),
                                art::ProductID()));
  std::cerr
    << "Group: type of group = "
    << cet::demangle_symbol(typeid(*g.get()).name())
    << "\n";
  BOOST_REQUIRE((!dynamic_cast<art::AssnsGroup*>(g.get())));
}

BOOST_AUTO_TEST_CASE(AssnsGroup)
{
  std::unique_ptr<art::Group>
    g(art::gfactory::make_group(makeBranchDescription(art::InEvent,
                                                      "AgGen",
                                                      "GEN",
                                                      "AG1",
                                                      fhicl::ParameterSet(),
                                                      art::TypeID(typeid(art::Assns<size_t, std::string>))),
                                art::ProductID()));
  std::cerr
    << "AssnsGroup: type of group = "
    << cet::demangle_symbol(typeid(*g.get()).name())
    << "\n";
  BOOST_REQUIRE((dynamic_cast<art::AssnsGroup*>(g.get())));
}

BOOST_AUTO_TEST_SUITE_END()
