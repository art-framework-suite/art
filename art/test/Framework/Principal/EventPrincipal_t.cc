/*----------------------------------------------------------------------

  Test of the EventPrincipal class.

  ----------------------------------------------------------------------*/

#define BOOST_TEST_MODULE (eventprincipal_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

using namespace std::string_literals;

class MPRGlobalTestFixture {
public:
  MPRGlobalTestFixture();

  std::map<std::string, art::BranchKey> branchKeys_{};
  std::map<std::string, art::ProcessConfiguration*> processConfigurations_{};

private:
  art::ProcessConfiguration* fake_single_module_process(
    std::string const& tag,
    std::string const& processName,
    fhicl::ParameterSet const& moduleParams,
    std::string const& release = art::getReleaseVersion());

  art::BranchDescription fake_single_process_branch(
    std::string const& tag,
    std::string const& processName,
    std::string const& productInstanceName = {});

  art::MasterProductRegistry productRegistry_{};
};

MPRGlobalTestFixture::MPRGlobalTestFixture()
{
  // We can only insert products registered in the MasterProductRegistry.
  std::vector<art::BranchDescription> descriptions;
  descriptions.push_back(fake_single_process_branch("hlt", "HLT"));
  descriptions.push_back(fake_single_process_branch("prod", "PROD"));
  descriptions.push_back(fake_single_process_branch("test", "TEST"));
  descriptions.push_back(fake_single_process_branch("user", "USER"));
  descriptions.push_back(fake_single_process_branch("rick", "USER2", "rick"));
  productRegistry_.addProductsFromModule(move(descriptions));
  productRegistry_.finalizeForProcessing();
  art::ProductMetaData::create_instance(productRegistry_);
}

art::ProcessConfiguration*
MPRGlobalTestFixture::fake_single_module_process(
  std::string const& tag,
  std::string const& processName,
  fhicl::ParameterSet const& moduleParams,
  std::string const& release)
{
  fhicl::ParameterSet processParams;
  processParams.put(processName, moduleParams);
  processParams.put("process_name", processName);

  auto* result =
    new art::ProcessConfiguration(processName, processParams.id(), release);
  processConfigurations_[tag] = result;
  return result;
}

art::BranchDescription
MPRGlobalTestFixture::fake_single_process_branch(
  std::string const& tag,
  std::string const& processName,
  std::string const& productInstanceName)
{
  std::string const moduleLabel{processName + "dummyMod"};
  std::string const moduleClass{"DummyModule"};
  art::TypeID const dummyType{typeid(arttest::DummyProduct)};
  fhicl::ParameterSet modParams;
  modParams.put("module_type", moduleClass);
  modParams.put("module_label", moduleLabel);
  art::ModuleDescription const mod{
    modParams.id(),
    moduleClass,
    moduleLabel,
    *fake_single_module_process(tag, processName, modParams)};

  art::BranchDescription const result{
    art::InEvent,
    art::TypeLabel{dummyType,
                   productInstanceName,
                   art::SupportsView<arttest::DummyProduct>::value,
                   false},
    mod};
  branchKeys_.emplace(tag, art::BranchKey{result});
  return result;
}

struct EventPrincipalTestFixture {
  EventPrincipalTestFixture();
  MPRGlobalTestFixture& gf();
  std::unique_ptr<art::EventPrincipal> pEvent_{nullptr};
};

EventPrincipalTestFixture::EventPrincipalTestFixture()
{
  (void)gf(); // Bootstrap MasterProductRegistry creation first time out.

  art::EventID const eventID{101, 87, 20};

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.

  // Put products we'll look for into the EventPrincipal.
  std::unique_ptr<art::EDProduct> product =
    std::make_unique<art::Wrapper<arttest::DummyProduct>>();

  std::string const tag{"rick"};
  auto i = gf().branchKeys_.find(tag);
  BOOST_REQUIRE(i != gf().branchKeys_.end());

  auto const& pmd = art::ProductMetaData::instance();
  auto it = pmd.productList().find(i->second);

  art::BranchDescription const& branchFromRegistry{it->second};
  auto const pid = branchFromRegistry.productID();
  BOOST_CHECK_EQUAL(pmd.inputTag(pid), branchFromRegistry.inputTag());

  auto entryDescriptionPtr = std::make_shared<art::Parentage>();
  auto productProvenancePtr = std::make_unique<art::ProductProvenance const>(
    pid, art::productstatus::present(), entryDescriptionPtr);

  auto* process = gf().processConfigurations_[tag];
  BOOST_REQUIRE(process);
  art::Timestamp const now{1234567UL};
  art::RunAuxiliary const runAux{eventID.run(), now, now};
  auto rp = std::make_unique<art::RunPrincipal>(runAux, *process, nullptr);
  art::SubRunAuxiliary const subRunAux{rp->run(), eventID.subRun(), now, now};
  auto srp =
    std::make_unique<art::SubRunPrincipal>(subRunAux, *process, nullptr);
  srp->setRunPrincipal(rp.get());
  art::EventAuxiliary const eventAux{eventID, now, true};
  pEvent_ = std::make_unique<art::EventPrincipal>(eventAux, *process, nullptr);
  pEvent_->setSubRunPrincipal(srp.get());
  pEvent_->put(
    std::move(product), branchFromRegistry, std::move(productProvenancePtr));

  BOOST_REQUIRE_EQUAL(pEvent_->size(), 1u);
}

MPRGlobalTestFixture&
EventPrincipalTestFixture::gf()
{
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}

BOOST_FIXTURE_TEST_SUITE(eventprincipal_t, EventPrincipalTestFixture)

BOOST_AUTO_TEST_CASE(failgetbyIdTest)
{
  auto const invalid = art::ProductID::invalid();
  auto const& h = pEvent_->getByProductID(invalid);
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbySelectorTest)
{
  // We don't put ProductIDs into the EventPrincipal, so that's a type
  // sure not to match any product.
  auto const& wrapped = art::WrappedTypeID::make<art::ProductID>();

  art::ProcessNameSelector const pnsel{"PROD"};
  auto const& h = pEvent_->getBySelector(wrapped, pnsel);
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbyLabelTest)
{
  // We don't put ProductIDs into the EventPrincipal, so that's a type
  // sure not to match any product.
  auto const& wrapped = art::WrappedTypeID::make<art::ProductID>();

  std::string const label{"this does not exist"};

  auto const& h = pEvent_->getByLabel(wrapped, label, ""s, ""s);
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetManyTest)
{
  // We don't put ProductIDs into the EventPrincipal, so that's a type
  // sure not to match any product.
  auto const& wrapped = art::WrappedTypeID::make<art::ProductID>();

  art::ProcessNameSelector const sel{"PROD"};
  auto const& query_results = pEvent_->getMany(wrapped, sel);
  BOOST_CHECK(query_results.empty());
}

BOOST_AUTO_TEST_CASE(failgetManybyTypeTest)
{
  // We don't put ProductIDs into the EventPrincipal, so that's a type
  // sure not to match any product.
  auto const& wrapped = art::WrappedTypeID::make<art::ProductID>();

  // getManyByType is achieved by providing a selector that matches
  // everything.
  auto const& query_results =
    pEvent_->getMany(wrapped, art::MatchAllSelector{});
  BOOST_CHECK(query_results.empty());
}

BOOST_AUTO_TEST_SUITE_END()
