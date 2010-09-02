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
#include "art/Persistency/Provenance/ParameterSetID.h"
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
#include "art/ParameterSet/ParameterSet.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Utilities/GlobalIdentifier.h"

class test_ep: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(test_ep);
  CPPUNIT_TEST(failgetbyIdTest);
  CPPUNIT_TEST(failgetbySelectorTest);
  CPPUNIT_TEST(failgetbyLabelTest);
  CPPUNIT_TEST(failgetManyTest);
  CPPUNIT_TEST(failgetbyTypeTest);
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
  void failgetbyTypeTest();
  void failgetManybyTypeTest();
  void failgetbyInvalidIdTest();
  void failgetProvenanceTest();

private:

  edm::ProcessConfiguration*
  fake_single_module_process(std::string const& tag,
			     std::string const& processName,
			     edm::ParameterSet const& moduleParams,
			     std::string const& release = edm::getReleaseVersion(),
			     std::string const& pass = edm::getPassID() );
  edm::BranchDescription*
  fake_single_process_branch(std::string const& tag,
			     std::string const& processName,
			     std::string const& productInstanceName = std::string() );

  std::map<std::string, edm::BranchDescription*>    branchDescriptions_;
  std::map<std::string, edm::ProcessConfiguration*> processConfigurations_;

  edm::ProductRegistry*      pProductRegistry_;
  edm::EventPrincipal*       pEvent_;
  std::vector<edm::ProductID> contained_products_;

  edm::EventID               eventID_;
};

//----------------------------------------------------------------------
// registration of the test so that the runner can find it

CPPUNIT_TEST_SUITE_REGISTRATION(test_ep);


//----------------------------------------------------------------------

edm::ProcessConfiguration*
test_ep::fake_single_module_process(std::string const& tag,
				    std::string const& processName,
				    edm::ParameterSet const& moduleParams,
				    std::string const& release,
				    std::string const& pass)
{
  edm::ParameterSet processParams;
  processParams.addParameter(processName, moduleParams);
  processParams.addParameter<std::string>("@process_name",
					  processName);

  edm::ProcessConfiguration* result =
    new edm::ProcessConfiguration(processName, processParams.id(), release, pass);
  processConfigurations_[tag] = result;
  return result;
}

edm::BranchDescription*
test_ep::fake_single_process_branch(std::string const& tag,
				    std::string const& processName,
				    std::string const& productInstanceName)
{
  edm::ModuleDescription mod;
  std::string moduleLabel = processName + "dummyMod";
  std::string moduleClass("DummyModule");
  edm::TypeID dummyType(typeid(edmtest::DummyProduct));
  std::string productClassName = dummyType.userClassName();
  std::string friendlyProductClassName = dummyType.friendlyClassName();
  edm::ParameterSet modParams;
  modParams.addParameter<std::string>("@module_type", moduleClass);
  modParams.addParameter<std::string>("@module_label", moduleLabel);
  mod.parameterSetID_ = modParams.id();
  mod.moduleName_ = moduleClass;
  mod.moduleLabel_ = moduleLabel;
  edm::ProcessConfiguration* process =
    fake_single_module_process(tag, processName, modParams);
  mod.processConfiguration_ = *process;

  edm::BranchDescription* result =
    new edm::BranchDescription(edm::InEvent,
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
  edm::BranchIDListHelper::clearRegistries();

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.
  eventID_ = edm::EventID(101, 20);

  // We can only insert products registered in the ProductRegistry.
  pProductRegistry_ = new edm::ProductRegistry;
  pProductRegistry_->addProduct(*fake_single_process_branch("hlt",  "HLT"));
  pProductRegistry_->addProduct(*fake_single_process_branch("prod", "PROD"));
  pProductRegistry_->addProduct(*fake_single_process_branch("test", "TEST"));
  pProductRegistry_->addProduct(*fake_single_process_branch("user", "USER"));
  pProductRegistry_->addProduct(*fake_single_process_branch("rick", "USER2", "rick"));
  pProductRegistry_->setFrozen();
  edm::BranchIDListHelper::updateRegistries(*pProductRegistry_);

  // Put products we'll look for into the EventPrincipal.
  {
    typedef edmtest::DummyProduct PRODUCT_TYPE;
    typedef edm::Wrapper<PRODUCT_TYPE> WDP;
    std::auto_ptr<edm::EDProduct>  product(new WDP(std::auto_ptr<PRODUCT_TYPE>(new PRODUCT_TYPE)));

    std::string tag("rick");
    assert(branchDescriptions_[tag]);
    edm::BranchDescription branch = *branchDescriptions_[tag];

    branch.init();

    edm::ProductRegistry::ProductList const& pl = pProductRegistry_->productList();
    edm::BranchKey const bk(branch);
    edm::ProductRegistry::ProductList::const_iterator it = pl.find(bk);

    const edm::ConstBranchDescription branchFromRegistry(it->second);

    boost::shared_ptr<edm::Parentage> entryDescriptionPtr(new edm::Parentage);
    std::auto_ptr<edm::ProductProvenance> branchEntryInfoPtr(
      new edm::ProductProvenance(branchFromRegistry.branchID(),
                               edm::productstatus::present(),
                               entryDescriptionPtr));

    edm::ProcessConfiguration* process = processConfigurations_[tag];
    assert(process);
    std::string uuid = edm::createGlobalIdentifier();
    edm::Timestamp now(1234567UL);
    boost::shared_ptr<edm::ProductRegistry const> preg(pProductRegistry_);
    edm::RunAuxiliary runAux(eventID_.run(), now, now);
    boost::shared_ptr<edm::RunPrincipal> rp(new edm::RunPrincipal(runAux, preg, *process));
    edm::SubRunAuxiliary lumiAux(rp->run(), 1, now, now);
    boost::shared_ptr<edm::SubRunPrincipal>lbp(new edm::SubRunPrincipal(lumiAux, preg, *process));
    lbp->setRunPrincipal(rp);
    edm::EventAuxiliary eventAux(eventID_, uuid, now, lbp->luminosityBlock(), true);
    pEvent_ = new edm::EventPrincipal(eventAux, preg, *process);
    pEvent_->setSubRunPrincipal(lbp);
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
  edm::ProductID invalid;
  CPPUNIT_ASSERT_THROW(pEvent_->getByProductID(invalid), edm::Exception);

  edm::ProductID notpresent(0, 10000);
  edm::BasicHandle h(pEvent_->getByProductID(notpresent));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetbySelectorTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  edm::ProductID dummy;
  edm::TypeID tid(dummy);

  edm::ProcessNameSelector pnsel("PROD");
  edm::BasicHandle h(pEvent_->getBySelector(tid, pnsel));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetbyLabelTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  edm::ProductID dummy;
  edm::TypeID tid(dummy);

  std::string label("this does not exist");

  edm::BasicHandle h(pEvent_->getByLabel(tid, label, std::string(), std::string()));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetManyTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  edm::ProductID dummy;
  edm::TypeID tid(dummy);

  edm::ProcessNameSelector sel("PROD");
  std::vector<edm::BasicHandle > handles;
  pEvent_->getMany(tid, sel, handles);
  CPPUNIT_ASSERT(handles.empty());
}

void test_ep::failgetbyTypeTest()
{
  edm::ProductID dummy;
  edm::TypeID tid(dummy);
  edm::BasicHandle h(pEvent_->getByType(tid));
  CPPUNIT_ASSERT(h.failedToGet());
}

void test_ep::failgetManybyTypeTest()
{
  // We don't put ProductIDs into the EventPrincipal,
  // so that's a type sure not to match any product.
  edm::ProductID dummy;
  edm::TypeID tid(dummy);
  std::vector<edm::BasicHandle > handles;


  pEvent_->getManyByType(tid, handles);
  CPPUNIT_ASSERT(handles.empty());
}

void test_ep::failgetbyInvalidIdTest()
{
  //put_a_dummy_product("HLT");
  //put_a_product<edmtest::DummyProduct>(pProdConfig_, label);

  edm::ProductID id;
  CPPUNIT_ASSERT_THROW(pEvent_->getByProductID(id), edm::Exception);
}

void test_ep::failgetProvenanceTest()
{
  edm::BranchID id;
  CPPUNIT_ASSERT_THROW(pEvent_->getProvenance(id), edm::Exception);
}

