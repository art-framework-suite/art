/*----------------------------------------------------------------------

Test of the EventPrincipal class.

----------------------------------------------------------------------*/
#include <string>
#include <iostream>

#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Utilities/GlobalIdentifier.h"
#include "art/Utilities/TypeID.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Common/Wrapper.h"
#include "test/TestObjects/ToyProducts.h"

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"

#include "art/Framework/Core/GenericHandle.h"
#include <cppunit/extensions/HelperMacros.h>

// This is a gross hack, to allow us to test the event
namespace art
{
   class EDProducer
      {
      public:
         static void commitEvent(Event& e) { e.commit_(); }

      };
}

class testGenericHandle: public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(testGenericHandle);
CPPUNIT_TEST(failgetbyLabelTest);
CPPUNIT_TEST(getbyLabelTest);
CPPUNIT_TEST(failWrongType);
CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}
  void failgetbyLabelTest();
  void failWrongType();
  void getbyLabelTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testGenericHandle);

void testGenericHandle::failWrongType() {
   try {
      //intentionally misspelled type
      art::GenericHandle h("arttest::DmmyProduct");
      CPPUNIT_ASSERT("Failed to thow"==0);
   }
   catch (cet::exception& x) {
      // nothing to do
   }
   catch (...) {
      CPPUNIT_ASSERT("Threw wrong kind of exception" == 0);
   }
}
void testGenericHandle::failgetbyLabelTest() {

  art::EventID id;
  art::Timestamp time;
  std::string uuid = art::createGlobalIdentifier();
  art::ProcessConfiguration pc("PROD", fhicl::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  boost::shared_ptr<art::ProductRegistry const> preg(new art::ProductRegistry);
  art::RunAuxiliary runAux(id.run(), time, time);
  boost::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, preg, pc));
  art::SubRunAuxiliary subRunAux(rp->run(), 1, time, time);
  boost::shared_ptr<art::SubRunPrincipal>lbp(new art::SubRunPrincipal(subRunAux, preg, pc));
  lbp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(id, uuid, time, lbp->subRun(), true);
  art::EventPrincipal ep(eventAux, preg, pc);
  ep.setSubRunPrincipal(lbp);
  art::GenericHandle h("arttest::DummyProduct");
  bool didThrow=true;
  try {
     art::ModuleDescription modDesc;
     modDesc.moduleName_="Blah";
     modDesc.moduleLabel_="blahs";
     art::Event event(ep, modDesc);

     std::string label("this does not exist");
     event.getByLabel(label,h);
     *h;
     didThrow=false;
  }
  catch (cet::exception& x) {
    // nothing to do
  }
  catch (std::exception& x) {
    std::cout <<"caught std exception "<<x.what()<<std::endl;
    CPPUNIT_ASSERT("Threw std::exception!"==0);
  }
  catch (...) {
    CPPUNIT_ASSERT("Threw wrong kind of exception" == 0);
  }
  if( !didThrow) {
    CPPUNIT_ASSERT("Failed to throw required exception" == 0);
  }

}

void testGenericHandle::getbyLabelTest() {
  std::string processName = "PROD";

  typedef arttest::DummyProduct DP;
  typedef art::Wrapper<DP> WDP;
  std::auto_ptr<DP> pr(new DP);
  std::auto_ptr<art::EDProduct> pprod(new WDP(pr));
  std::string label("fred");
  std::string productInstanceName("Rick");

  arttest::DummyProduct dp;
  art::TypeID dummytype(dp);
  std::string className = dummytype.friendlyClassName();

  art::ModuleDescription modDesc;
  modDesc.moduleName_ = "Blah";
  modDesc.parameterSetID_ = fhicl::ParameterSet().id();

  art::BranchDescription product(art::InEvent,
				 label,
				 processName,
				 dummytype.userClassName(),
				 className,
				 productInstanceName,
				 modDesc
				);

  product.init();

  art::ProductRegistry *preg = new art::ProductRegistry;
  preg->addProduct(product);
  preg->setFrozen();
  art::BranchIDListHelper::updateRegistries(*preg);

  art::ProductRegistry::ProductList const& pl = preg->productList();
  art::BranchKey const bk(product);
  art::ProductRegistry::ProductList::const_iterator it = pl.find(bk);

  art::EventID col(1L, 1L);
  art::Timestamp fakeTime;
  std::string uuid = art::createGlobalIdentifier();
  art::ProcessConfiguration pc("PROD", fhicl::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  boost::shared_ptr<art::ProductRegistry const> pregc(preg);
  art::RunAuxiliary runAux(col.run(), fakeTime, fakeTime);
  boost::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, pregc, pc));
  art::SubRunAuxiliary subRunAux(rp->run(), 1, fakeTime, fakeTime);
  boost::shared_ptr<art::SubRunPrincipal>lbp(new art::SubRunPrincipal(subRunAux, pregc, pc));
  lbp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(col, uuid, fakeTime, lbp->subRun(), true);
  art::EventPrincipal ep(eventAux, pregc, pc);
  ep.setSubRunPrincipal(lbp);
  const art::BranchDescription& branchFromRegistry = it->second;
  boost::shared_ptr<art::Parentage> entryDescriptionPtr(new art::Parentage);
  std::auto_ptr<art::ProductProvenance> branchEntryInfoPtr(
      new art::ProductProvenance(branchFromRegistry.branchID(),
                              art::productstatus::present(),
                              entryDescriptionPtr));
  art::ConstBranchDescription const desc(branchFromRegistry);
  ep.put(pprod, desc, branchEntryInfoPtr);

  art::GenericHandle h("arttest::DummyProduct");
  try {
    art::ModuleDescription modDesc;
    modDesc.moduleName_="Blah";
    modDesc.moduleLabel_="blahs";
    art::Event event(ep, modDesc);

    event.getByLabel(label, productInstanceName,h);
  }
  catch (cet::exception& x) {
    std::cerr << x.explain_self()<< std::endl;
    CPPUNIT_ASSERT("Threw cet::exception unexpectedly" == 0);
  }
  catch(std::exception& x){
     std::cerr <<x.what()<<std::endl;
     CPPUNIT_ASSERT("threw std::exception"==0);
  }
  catch (...) {
    std::cerr << "Unknown exception type\n";
    CPPUNIT_ASSERT("Threw exception unexpectedly" == 0);
  }
  CPPUNIT_ASSERT(h.isValid());
  CPPUNIT_ASSERT(h.provenance()->moduleLabel() == label);
}
