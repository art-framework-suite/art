/*----------------------------------------------------------------------

Test of the EventPrincipal class.

----------------------------------------------------------------------*/
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <cppunit/extensions/HelperMacros.h>

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSetID.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/Provenance.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Common/Wrapper.h"
#include "test/TestObjects/ToyProducts.h"
#include "art/Persistency/Common/BasicHandle.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/Selector.h"
#include "art/Utilities/TypeID.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Framework/Core/RootDictionaryManager.h"

class test_ep: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(test_ep);
  CPPUNIT_TEST(failgetbyIdTest);
  CPPUNIT_TEST(failgetbySelectorTest);
  CPPUNIT_TEST(failgetbyLabelTest);
  CPPUNIT_TEST(failgetManyTest);
  CPPUNIT_TEST(failgetManybyTypeTest);
  CPPUNIT_TEST(failgetbyInvalidIdTest);
  CPPUNIT_TEST(failgetProvenanceTest);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void tearDown();
  void failgetbyIdTest();
  void failgetbySelectorTest();
  void failgetbyLabelTest();
  void failgetManyTest();
  void failgetManybyTypeTest();
  void failgetbyInvalidIdTest();
  void failgetProvenanceTest();

private:
  art::ProcessConfiguration*
  fake_single_module_process(std::string const& tag,
			     std::string const& processName,
			     fhicl::ParameterSet const& moduleParams,
			     std::string const& release = art::getReleaseVersion(),
			     std::string const& pass = art::getPassID() );
  art::BranchDescription*
  fake_single_process_branch(std::string const& tag,
			     std::string const& processName,
			     std::string const& productInstanceName = std::string() );

  std::map<std::string, art::BranchDescription*>    branchDescriptions_;
  std::map<std::string, art::ProcessConfiguration*> processConfigurations_;

  art::ProductRegistry*      pProductRegistry_;
  art::EventPrincipal*       pEvent_;
  std::vector<art::ProductID> contained_products_;

  art::EventID               eventID_;
  art::RootDictionaryManager rdm_;
};

//----------------------------------------------------------------------
// registration of the test so that the runner can find it

CPPUNIT_TEST_SUITE_REGISTRATION(test_ep);


//----------------------------------------------------------------------

art::ProcessConfiguration*
test_ep::fake_single_module_process(std::string const& tag,
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

art::BranchDescription*
test_ep::fake_single_process_branch(std::string const& tag,
				    std::string const& processName,
				    std::string const& productInstanceName)
{
  art::ModuleDescription mod;
  std::string moduleLabel = processName + "dummyMod";
  std::string moduleClass("DummyModule");
  art::TypeID dummyType(typeid(arttest::DummyProduct));
  std::string productClassName = dummyType.userClassName();
  std::string friendlyProductClassName = dummyType.friendlyClassName();
  fhicl::ParameterSet modParams;
  modParams.put<std::string>("module_type", moduleClass);
  modParams.put<std::string>("module_label", moduleLabel);
  mod.parameterSetID_ = modParams.id();
  mod.moduleName_ = moduleClass;
  mod.moduleLabel_ = moduleLabel;
  art::ProcessConfiguration* process =
    fake_single_module_process(tag, processName, modParams);
  mod.processConfiguration_ = *process;

  art::BranchDescription* result =
    new art::BranchDescription(art::InEvent,
			       moduleLabel,
			       processName,
			       productClassName,
			       friendlyProductClassName,
			       productInstanceName,
			       mod);
  branchDescriptions_[tag] = result;
  return result;
}

void test_ep::setUp()
{
  art::BranchIDListHelper::clearRegistries();

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.
  eventID_ = art::EventID(101, 87, 20);

  // We can only insert products registered in the ProductRegistry.
  pProductRegistry_ = new art::ProductRegistry;
  pProductRegistry_->addProduct(*fake_single_process_branch("hlt",  "HLT"));
  pProductRegistry_->addProduct(*fake_single_process_branch("prod", "PROD"));
  pProductRegistry_->addProduct(*fake_single_process_branch("test", "TEST"));
  pProductRegistry_->addProduct(*fake_single_process_branch("user", "USER"));
  pProductRegistry_->addProduct(*fake_single_process_branch("rick", "USER2", "rick"));
  pProductRegistry_->setFrozen();
  art::BranchIDListHelper::updateRegistries(*pProductRegistry_);

  // Put products we'll look for into the EventPrincipal.
  {
    typedef arttest::DummyProduct PRODUCT_TYPE;
    typedef art::Wrapper<PRODUCT_TYPE> WDP;
    std::auto_ptr<art::EDProduct>  product(new WDP(std::auto_ptr<PRODUCT_TYPE>(new PRODUCT_TYPE)));

    std::string tag("rick");
    assert(branchDescriptions_[tag]);
    art::BranchDescription branch = *branchDescriptions_[tag];

    branch.init();

    art::ProductRegistry::ProductList const& pl = pProductRegistry_->productList();
    art::BranchKey const bk(branch);
    art::ProductRegistry::ProductList::const_iterator it = pl.find(bk);

    const art::ConstBranchDescription branchFromRegistry(it->second);

    boost::shared_ptr<art::Parentage> entryDescriptionPtr(new art::Parentage);
    std::auto_ptr<art::ProductProvenance> branchEntryInfoPtr(
      new art::ProductProvenance(branchFromRegistry.branchID(),
                               art::productstatus::present(),
                               entryDescriptionPtr));

    art::ProcessConfiguration* process = processConfigurations_[tag];
    assert(process);
    art::Timestamp now(1234567UL);
    cet::exempt_ptr<art::ProductRegistry const> preg(pProductRegistry_);
    art::RunAuxiliary runAux(eventID_.run(), now, now);
    boost::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, preg, *process));
    art::SubRunAuxiliary subRunAux(rp->run(), eventID_.subRun(), now, now);
    boost::shared_ptr<art::SubRunPrincipal>srp(new art::SubRunPrincipal(subRunAux, preg, *process));
    srp->setRunPrincipal(rp);
    art::EventAuxiliary eventAux(eventID_, now, true);
    pEvent_ = new art::EventPrincipal(eventAux, preg, *process);
    pEvent_->setSubRunPrincipal(srp);
    pEvent_->put(product, branchFromRegistry, branchEntryInfoPtr);
  }
  CPPUNIT_ASSERT(pEvent_->size() == 1);

}

template <class MAP>
void clear_map(MAP& m)
{
  for (typename MAP::iterator i = m.begin(), e = m.end(); i != e; ++i)
    delete i->second;
}

void test_ep::tearDown()
{

  clear_map(branchDescriptions_);
  clear_map(processConfigurations_);

  delete pEvent_;
  pEvent_ = 0;

  pProductRegistry_ = 0;

}


//----------------------------------------------------------------------
// Test functions
//----------------------------------------------------------------------

void test_ep::failgetbyIdTest()
{
  art::ProductID invalid;
  CPPUNIT_ASSERT_THROW(pEvent_->getByProductID(invalid), art::Exception);

  art::ProductID notpresent(0, 10000);
  art::BasicHandle h(pEvent_->getByProductID(notpresent));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetbySelectorTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  art::ProcessNameSelector pnsel("PROD");
  art::BasicHandle h(pEvent_->getBySelector(tid, pnsel));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetbyLabelTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  std::string label("this does not exist");

  art::BasicHandle h(pEvent_->getByLabel(tid, label, std::string(), std::string()));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetManyTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);

  art::ProcessNameSelector sel("PROD");
  std::vector<art::BasicHandle > handles;
  pEvent_->getMany(tid, sel, handles);
  CPPUNIT_ASSERT(handles.empty());
}

void test_ep::failgetManybyTypeTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  art::ProductID dummy;
  art::TypeID tid(dummy);
  std::vector<art::BasicHandle > handles;


  pEvent_->getManyByType(tid, handles);
  CPPUNIT_ASSERT(handles.empty());
}

void test_ep::failgetbyInvalidIdTest()
{
  //put_a_dummy_product("HLT");
  //put_a_product<arttest::DummyProduct>(pProdConfig_, label);

  art::ProductID id;
  CPPUNIT_ASSERT_THROW(pEvent_->getByProductID(id), art::Exception);
}

void test_ep::failgetProvenanceTest()
{
  art::BranchID id;
  CPPUNIT_ASSERT_THROW(pEvent_->getProvenance(id), art::Exception);
}

