// Test of GroupFactory functions

#define BOOST_TEST_MODULE (GroupFactory_t)
#include "cetlib/quiet_unit_test.hpp"

#include <iostream>
#include <typeinfo>
#include <vector>

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace {
  template <typename PROD>
  art::BranchDescription
  makeBranchDescription(art::BranchType bt,
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
        art::ProcessConfiguration{
          processName, fhicl::ParameterSet{}.id(), art::getReleaseVersion()}}};
  }
}

struct TestFixture {
  TestFixture() {}
};

BOOST_FIXTURE_TEST_SUITE(GroupFactory_t, TestFixture)

BOOST_AUTO_TEST_CASE(Group)
{
  std::unique_ptr<art::Group> g(art::gfactory::make_group(
    makeBranchDescription<int>(
      art::InEvent, "GGen", "GEN", "G1", fhicl::ParameterSet{}),
    art::ProductID{},
    art::RangeSet::invalid()));
  std::cerr << "Group: type of group = "
            << cet::demangle_symbol(typeid(decltype(*g.get())).name()) << "\n";
  BOOST_REQUIRE((!dynamic_cast<art::AssnsGroup*>(g.get())));
}

BOOST_AUTO_TEST_CASE(AssnsGroup)
{
  std::unique_ptr<art::Group> g(art::gfactory::make_group(
    makeBranchDescription<art::Assns<size_t, std::string>>(
      art::InEvent, "AgGen", "GEN", "AG1", fhicl::ParameterSet{}),
    art::ProductID{},
    art::RangeSet::invalid()));
  std::cerr << "AssnsGroup: type of group = "
            << cet::demangle_symbol(typeid(decltype(*g.get())).name()) << "\n";
  BOOST_REQUIRE((dynamic_cast<art::AssnsGroup*>(g.get())));
}

BOOST_AUTO_TEST_SUITE_END()
