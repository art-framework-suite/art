// Test of GroupFactory functions

#define BOOST_TEST_MODULE (GroupFactory_t)
#include "boost/test/auto_unit_test.hpp"

#include <iostream>
#include <typeinfo>
#include <vector>

#include "Reflex/Type.h"

#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace {
  void checkTypeAndWrapper(art::TypeID const &producedType) {
    BOOST_REQUIRE(producedType.hasDictionary());
    BOOST_REQUIRE(Reflex::Type::ByName(art::wrappedClassName(producedType.className())));
  }
  art::BranchDescription
  makeBranchDescription(art::BranchType bt,
                        std::string const &moduleLabel,
                        std::string const &processName,
                        std::string const &instanceName,
                        fhicl::ParameterSet const &pset,
                        art::TypeID const &producedType) {
    checkTypeAndWrapper(producedType);
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
    static art::RootDictionaryManager rdm_s;
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
