/*----------------------------------------------------------------------

Test of the EventPrincipal class.

----------------------------------------------------------------------*/

#define BOOST_TEST_MODULE ( eventprincipal_t )
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/Principal/EventPrincipal.h"

#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "art/Utilities/TypeID.h"
#include "art/Version/GetReleaseVersion.h"
#include <memory>
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "test/TestObjects/ToyProducts.h"
#include <map>
#include <stdexcept>
#include <string>
#include <typeinfo>

class MPRGlobalTestFixture {
public:
  MPRGlobalTestFixture();

  typedef std::map<std::string, art::BranchKey> BKmap_t;

  BKmap_t branchKeys_;
  std::map<std::string, art::ProcessConfiguration*> processConfigurations_;

private:
  art::ProcessConfiguration*
  fake_single_module_process(std::string const& tag,
                             std::string const& processName,
                             fhicl::ParameterSet const& moduleParams,
                             std::string const& release = art::getReleaseVersion(),
                             std::string const& pass = art::getPassID() );

  std::unique_ptr<art::BranchDescription>
  fake_single_process_branch(std::string const& tag,
                             std::string const& processName,
                             std::string const& productInstanceName = std::string() );

  art::MasterProductRegistry  productRegistry_;
};


MPRGlobalTestFixture::MPRGlobalTestFixture()
  :
  branchKeys_(),
  processConfigurations_(),
  productRegistry_()
{
  // We can only insert products registered in the MasterProductRegistry.
  productRegistry_.addProduct(fake_single_process_branch("hlt",  "HLT"));
  productRegistry_.addProduct(fake_single_process_branch("prod", "PROD"));
  productRegistry_.addProduct(fake_single_process_branch("test", "TEST"));
  productRegistry_.addProduct(fake_single_process_branch("user", "USER"));
  productRegistry_.addProduct(fake_single_process_branch("rick", "USER2", "rick"));
  productRegistry_.setFrozen();
  art::BranchIDListHelper::updateRegistries(productRegistry_);
  art::ProductMetaData::create_instance(productRegistry_);
}

art::ProcessConfiguration*
MPRGlobalTestFixture::
fake_single_module_process(std::string const& tag,
                           std::string const& processName,
                           fhicl::ParameterSet const& moduleParams,
                           std::string const& release,
                           std::string const& pass)
{
  fhicl::ParameterSet processParams;
  processParams.put(processName, moduleParams);
  processParams.put<std::string>("process_name",
                                          processName);

  art::ProcessConfiguration* result =
    new art::ProcessConfiguration(processName, processParams.id(), release, pass);
  processConfigurations_[tag] = result;
  return result;
}

std::unique_ptr<art::BranchDescription>
MPRGlobalTestFixture::
fake_single_process_branch(std::string const& tag,
                           std::string const& processName,
                           std::string const& productInstanceName)
{
  std::string moduleLabel = processName + "dummyMod";
  std::string moduleClass("DummyModule");
  art::TypeID dummyType(typeid(arttest::DummyProduct));
  fhicl::ParameterSet modParams;
  modParams.put<std::string>("module_type", moduleClass);
  modParams.put<std::string>("module_label", moduleLabel);
  art::ModuleDescription mod(modParams.id(),
                             moduleClass,
                             moduleLabel,
                             *fake_single_module_process(tag, processName, modParams));

  art::BranchDescription* result =
    new art::BranchDescription(art::TypeLabel(art::InEvent,
                                              dummyType,
                                              productInstanceName),
                               mod);
  branchKeys_.insert(std::make_pair(tag, art::BranchKey(*result)));
  return std::unique_ptr<art::BranchDescription>(result);
}

struct EventPrincipalTestFixture {
  typedef MPRGlobalTestFixture::BKmap_t BKmap_t;

  EventPrincipalTestFixture();

  MPRGlobalTestFixture &gf();

  std::unique_ptr<art::EventPrincipal> pEvent_;
};

EventPrincipalTestFixture::EventPrincipalTestFixture()
  :
  pEvent_()
{
  (void) gf(); // Bootstrap MasterProductRegistry creation first time out.

  art::EventID eventID(101, 87, 20);

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.

  // Put products we'll look for into the EventPrincipal.
  typedef arttest::DummyProduct PRODUCT_TYPE;
  typedef art::Wrapper<PRODUCT_TYPE> WDP;
  std::unique_ptr<art::EDProduct>  product(new WDP(std::unique_ptr<PRODUCT_TYPE>(new PRODUCT_TYPE)));

  std::string tag("rick");
  BKmap_t::const_iterator i(gf().branchKeys_.find(tag));
  BOOST_REQUIRE( i != gf().branchKeys_.end());

  auto it = art::ProductMetaData::instance().productList().find(i->second);

  art::BranchDescription const& branchFromRegistry(it->second);

  std::shared_ptr<art::Parentage> entryDescriptionPtr(new art::Parentage);
  std::unique_ptr<art::ProductProvenance const>
    productProvenancePtr(new art::ProductProvenance(branchFromRegistry.branchID(),
                                                    art::productstatus::present(),
                                                    entryDescriptionPtr));

  art::ProcessConfiguration* process = gf().processConfigurations_[tag];
  BOOST_REQUIRE(process);
  art::Timestamp now(1234567UL);
  art::RunAuxiliary runAux(eventID.run(), now, now);
  std::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, *process));
  art::SubRunAuxiliary subRunAux(rp->run(), eventID.subRun(), now, now);
  std::shared_ptr<art::SubRunPrincipal>srp(new art::SubRunPrincipal(subRunAux, *process));
  srp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(eventID, now, true);
  pEvent_.reset(new art::EventPrincipal(eventAux, *process));
  pEvent_->setSubRunPrincipal(srp);
  pEvent_->put(std::move(product), branchFromRegistry, std::move(productProvenancePtr));

  BOOST_REQUIRE_EQUAL(pEvent_->size(), 1u);
}

MPRGlobalTestFixture &
EventPrincipalTestFixture::gf() {
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}

BOOST_FIXTURE_TEST_SUITE(eventprincipal_t, EventPrincipalTestFixture)

BOOST_AUTO_TEST_CASE(failgetbyIdTest)
{
  art::ProductID invalid;
  BOOST_CHECK_THROW(pEvent_->getByProductID(invalid), art::Exception);

  art::ProductID notpresent(0, 10000);
  art::GroupQueryResult h(pEvent_->getByProductID(notpresent));
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbySelectorTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  art::ProcessNameSelector pnsel("PROD");
  art::GroupQueryResult h(pEvent_->getBySelector(tid, pnsel));
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetbyLabelTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  std::string label("this does not exist");

  art::GroupQueryResult h(pEvent_->getByLabel(tid, label, std::string(), std::string()));
  BOOST_CHECK(h.failed());
}

BOOST_AUTO_TEST_CASE(failgetManyTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  art::ProcessNameSelector sel("PROD");
  std::vector<art::GroupQueryResult > handles;
  pEvent_->getMany(tid, sel, handles);
  BOOST_CHECK(handles.empty());
}

BOOST_AUTO_TEST_CASE(failgetManybyTypeTest)
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);
  std::vector<art::GroupQueryResult > handles;


  pEvent_->getManyByType(tid, handles);
  BOOST_CHECK(handles.empty());
}

BOOST_AUTO_TEST_CASE(failgetbyInvalidIdTest)
{
  art::ProductID id;
  BOOST_CHECK_THROW(pEvent_->getByProductID(id), art::Exception);
}

BOOST_AUTO_TEST_SUITE_END()
