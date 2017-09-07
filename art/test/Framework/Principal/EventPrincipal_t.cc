// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE ( eventprincipal_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/ModuleType.h"
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

using namespace std;
using namespace std::string_literals;
using namespace art;

class MPRGlobalTestFixture {

public: // TYPES

  using BKmap_t = map<string, BranchKey>;

public: // MEMBER FUNCTIONS -- Special Member Functions

  MPRGlobalTestFixture();

private: // MEMBER FUNCTIONS -- Implementation details

  ProcessConfiguration*
  fake_single_module_process(string const& tag, string const& processName, fhicl::ParameterSet const& moduleParams,
                             string const& release = getReleaseVersion());

  unique_ptr<BranchDescription>
  fake_single_process_branch(string const& tag, string const& processName, string const& productInstanceName = {});

public: // MEMBER DATA

  BKmap_t
  branchKeys_{};

  map<string, ProcessConfiguration*>
  processConfigurations_{};

private: // MEMBER DATA

  MasterProductRegistry
  productRegistry_{};

};


MPRGlobalTestFixture::
MPRGlobalTestFixture()
{
  // We can only insert products registered in the MasterProductRegistry.
  productRegistry_.addProduct(fake_single_process_branch("hlt",  "HLT"));
  productRegistry_.addProduct(fake_single_process_branch("prod", "PROD"));
  productRegistry_.addProduct(fake_single_process_branch("test", "TEST"));
  productRegistry_.addProduct(fake_single_process_branch("user", "USER"));
  productRegistry_.addProduct(fake_single_process_branch("rick", "USER2", "rick"));
  productRegistry_.finalizeForProcessing();
  ProductMetaData::create_instance(productRegistry_);
}

ProcessConfiguration*
MPRGlobalTestFixture::
fake_single_module_process(string const& tag, string const& processName, fhicl::ParameterSet const& moduleParams,
                           string const& release)
{
  fhicl::ParameterSet processParams;
  processParams.put(processName, moduleParams);
  processParams.put("process_name", processName);
  auto* result = new ProcessConfiguration(processName, processParams.id(), release);
  processConfigurations_[tag] = result;
  return result;
}

unique_ptr<BranchDescription>
MPRGlobalTestFixture::
fake_single_process_branch(string const& tag, string const& processName, string const& productInstanceName)
{
  string const moduleLabel{processName + "dummyMod"};
  string const moduleClass{"DummyModule"};
  TypeID const dummyType{typeid(arttest::DummyProduct)};
  fhicl::ParameterSet modParams;
  modParams.put("module_type", moduleClass);
  modParams.put("module_label", moduleLabel);
  ModuleDescription const mod(modParams.id(), moduleClass, moduleLabel, static_cast<int>(ModuleThreadingType::LEGACY),
                              *fake_single_module_process(tag, processName, modParams));
  auto* result = new BranchDescription(InEvent, TypeLabel{dummyType, productInstanceName}, mod);
  branchKeys_.insert(make_pair(tag, BranchKey(*result)));
  return unique_ptr<BranchDescription>(result);
}

struct EventPrincipalTestFixture {

  using BKmap_t = MPRGlobalTestFixture::BKmap_t;

  EventPrincipalTestFixture();

  MPRGlobalTestFixture&
  gf();

  unique_ptr<EventPrincipal>
  pEvent_{nullptr};

};

EventPrincipalTestFixture::
EventPrincipalTestFixture()
{
  (void) gf(); // Bootstrap MasterProductRegistry creation first time out.
  EventID eventID(101, 87, 20);
  // Making a functional EventPrincipal is not trivial, so we do it all here.
  // Put products we'll look for into the EventPrincipal.
  typedef arttest::DummyProduct PRODUCT_TYPE;
  typedef Wrapper<PRODUCT_TYPE> WDP;
  unique_ptr<EDProduct> product{new WDP(unique_ptr<PRODUCT_TYPE>(new PRODUCT_TYPE))};
  string tag("rick");
  BKmap_t::const_iterator i(gf().branchKeys_.find(tag));
  BOOST_REQUIRE(i != gf().branchKeys_.end());
  auto it = ProductMetaData::instance().productList().find(i->second);
  BranchDescription const& bd(it->second);
  auto entryDescriptionPtr = make_shared<Parentage>();
  auto productProvenancePtr = make_unique<ProductProvenance const>(bd.productID(), productstatus::present(), entryDescriptionPtr->parents());
  ProcessConfiguration* process = gf().processConfigurations_[tag];
  BOOST_REQUIRE(process);
  Timestamp now(1234567UL);
  RunAuxiliary runAux {eventID.run(), now, now};
  auto rp = make_unique<RunPrincipal>(runAux, *process);
  SubRunAuxiliary subRunAux {rp->run(), eventID.subRun(), now, now};
  auto srp = make_unique<SubRunPrincipal>(subRunAux, *process);
  srp->setRunPrincipal(rp.get());
  EventAuxiliary eventAux(eventID, now, true);
  pEvent_ = make_unique<EventPrincipal>(eventAux, *process);
  pEvent_->setSubRunPrincipal(srp.get());
  pEvent_->put(bd, move(productProvenancePtr), move(product), move(make_unique<RangeSet>()));
  BOOST_REQUIRE_EQUAL(pEvent_->size(), 5u);
}

MPRGlobalTestFixture&
EventPrincipalTestFixture::
gf()
{
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}

BOOST_FIXTURE_TEST_SUITE(eventprincipal_t, EventPrincipalTestFixture)

BOOST_AUTO_TEST_CASE(failgetbyIdTest)
{
  auto const invalid = ProductID::invalid();
  GroupQueryResult h(pEvent_->getByProductID(invalid));
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbySelectorTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  ProductID const dummy;
  TypeID const tid{dummy};
  ProcessNameSelector pnsel("PROD");
  GroupQueryResult h(pEvent_->getBySelector(tid, pnsel));
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbyLabelTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  ProductID const dummy;
  TypeID const tid{dummy};
  string label("this does not exist");
  GroupQueryResult const h{pEvent_->getByLabel(tid, label, ""s, ""s)};
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetManyTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  ProductID const dummy;
  TypeID const tid{dummy};
  ProcessNameSelector const sel{"PROD"};
  vector<GroupQueryResult> handles;
  pEvent_->getMany(tid, sel, handles);
  BOOST_CHECK(handles.empty());
}

BOOST_AUTO_TEST_CASE(failgetManybyTypeTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  ProductID const dummy;
  TypeID const tid{dummy};
  // getManyByType is achieved by providing a selector that matches
  // everything.
  vector<GroupQueryResult> handles;
  pEvent_->getMany(tid, MatchAllSelector{}, handles);
  BOOST_CHECK(handles.empty());
}

BOOST_AUTO_TEST_SUITE_END()
