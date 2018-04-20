// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (eventprincipal_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
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

using namespace std;
using namespace std::string_literals;
using namespace art;

class ProductTablesFixture {
public: // MEMBER FUNCTIONS -- Special Member Functions
  ProductTablesFixture();

  ProductTables producedProducts_{ProductTables::invalid()};
  std::map<std::string, art::ProductID> productIDs_{};
  std::map<std::string, art::ProcessConfiguration*> processConfigurations_{};

private:
  art::BranchDescription fake_single_process_branch(
    std::string const& tag,
    std::string const& processName,
    std::string const& productInstanceName = {});

  ProcessConfiguration* fake_single_module_process(
    std::string const& tag,
    std::string const& processName,
    fhicl::ParameterSet const& moduleParams,
    std::string const& release = art::getReleaseVersion());
};

ProductTablesFixture::ProductTablesFixture()
{
  // We can only insert products registered in the product tables
  ProductDescriptions descriptions;
  descriptions.push_back(fake_single_process_branch("hlt", "HLT"));
  descriptions.push_back(fake_single_process_branch("prod", "PROD"));
  descriptions.push_back(fake_single_process_branch("test", "TEST"));
  descriptions.push_back(fake_single_process_branch("user", "USER"));
  descriptions.push_back(fake_single_process_branch("rick", "USER2", "rick"));
  producedProducts_ = ProductTables{descriptions};
}

ProcessConfiguration*
ProductTablesFixture::fake_single_module_process(
  string const& tag,
  string const& processName,
  fhicl::ParameterSet const& moduleParams,
  string const& release)
{
  fhicl::ParameterSet processParams;
  processParams.put(processName, moduleParams);
  processParams.put("process_name", processName);
  auto result =
    new ProcessConfiguration(processName, processParams.id(), release);
  processConfigurations_[tag] = result;
  return result;
}

art::BranchDescription
ProductTablesFixture::fake_single_process_branch(
  std::string const& tag,
  std::string const& processName,
  std::string const& productInstanceName)
{
  string const moduleLabel{processName + "dummyMod"};
  string const moduleClass{"DummyModule"};
  TypeID const dummyType{typeid(arttest::DummyProduct)};
  fhicl::ParameterSet modParams;
  modParams.put("module_type", moduleClass);
  modParams.put("module_label", moduleLabel);

  art::ModuleDescription const mod{
    modParams.id(),
    moduleClass,
    moduleLabel,
    static_cast<int>(ModuleThreadingType::legacy),
    *fake_single_module_process(tag, processName, modParams)};

  art::BranchDescription const result{
    art::InEvent,
    art::TypeLabel{dummyType,
                   productInstanceName,
                   art::SupportsView<arttest::DummyProduct>::value,
                   false},
    mod};
  productIDs_.emplace(tag, result.productID());
  return result;
}

struct EventPrincipalTestFixture {
  EventPrincipalTestFixture();
  ProductTablesFixture& ptf();
  std::unique_ptr<art::EventPrincipal> pEvent_{nullptr};
};

EventPrincipalTestFixture::EventPrincipalTestFixture()
{
  (void)ptf(); // Bootstrap ProductTables creation first time out.
  EventID const eventID{101, 87, 20};

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.  Put products we'll look for into the EventPrincipal.
  std::unique_ptr<art::EDProduct> product =
    std::make_unique<art::Wrapper<arttest::DummyProduct>>();

  std::string const tag{"rick"};
  auto i = ptf().productIDs_.find(tag);
  BOOST_REQUIRE(i != ptf().productIDs_.end());

  auto pd = ptf().producedProducts_.get(InEvent).description(i->second);
  BOOST_REQUIRE(pd != nullptr);

  auto entryDescriptionPtr = std::make_shared<art::Parentage>();
  auto productProvenancePtr = std::make_unique<art::ProductProvenance const>(
    pd->productID(),
    art::productstatus::present(),
    entryDescriptionPtr->parents());

  auto* process = ptf().processConfigurations_[tag];
  BOOST_REQUIRE(process);

  constexpr art::Timestamp now{1234567UL};

  art::RunAuxiliary const runAux{eventID.run(), now, now};
  auto rp = std::make_unique<art::RunPrincipal>(runAux, *process, nullptr);

  art::SubRunAuxiliary const subRunAux{rp->run(), eventID.subRun(), now, now};
  auto srp =
    std::make_unique<art::SubRunPrincipal>(subRunAux, *process, nullptr);
  srp->setRunPrincipal(rp.get());

  art::EventAuxiliary const eventAux{eventID, now, true};
  pEvent_ = std::make_unique<art::EventPrincipal>(eventAux, *process, nullptr);
  pEvent_->setSubRunPrincipal(srp.get());
  pEvent_->createGroupsForProducedProducts(ptf().producedProducts_);
  pEvent_->enableLookupOfProducedProducts(ptf().producedProducts_);
  pEvent_->put(
    *pd, move(productProvenancePtr), move(product), make_unique<RangeSet>());
  BOOST_REQUIRE_EQUAL(pEvent_->size(), 5u);

  auto pdPtr = pEvent_->getProductDescription(i->second);
  BOOST_REQUIRE_EQUAL(*pd, *pdPtr);
}

ProductTablesFixture&
EventPrincipalTestFixture::ptf()
{
  static ProductTablesFixture ptf_s;
  return ptf_s;
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
