/*----------------------------------------------------------------------

Test of the EventPrincipal class.

----------------------------------------------------------------------*/
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "art/Utilities/EDMException.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/Utilities/GlobalIdentifier.h"
#include "art/Utilities/TypeID.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/Timestamp.h"
//#include "art/Framework/Core/Selector.h"
#include "test/TestObjects/ToyProducts.h"

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"

//have to do this evil in order to access commit_ member function
#define private public
#include "art/Framework/Core/Event.h"
#undef private

#include <cppunit/extensions/HelperMacros.h>

class testEventGetRefBeforePut: public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE(testEventGetRefBeforePut);
CPPUNIT_TEST(failGetProductNotRegisteredTest);
CPPUNIT_TEST(getRefTest);
CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}
  void failGetProductNotRegisteredTest();
  void getRefTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testEventGetRefBeforePut);

void testEventGetRefBeforePut::failGetProductNotRegisteredTest() {

  art::ProductRegistry *preg = new art::ProductRegistry;
  preg->setFrozen();
  art::BranchIDListHelper::updateRegistries(*preg);
  art::EventID col(1L, 1L);
  std::string uuid = art::createGlobalIdentifier();
  art::Timestamp fakeTime;
  art::ProcessConfiguration pc("PROD", art::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  boost::shared_ptr<art::ProductRegistry const> pregc(preg);
  art::RunAuxiliary runAux(col.run(), fakeTime, fakeTime);
  boost::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, pregc, pc));
  art::SubRunAuxiliary subRunAux(rp->run(), 1, fakeTime, fakeTime);
  boost::shared_ptr<art::SubRunPrincipal>lbp(new art::SubRunPrincipal(subRunAux, pregc, pc));
  lbp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(col, uuid, fakeTime, lbp->subRun(), true);
  art::EventPrincipal ep(eventAux, pregc, pc);
  ep.setSubRunPrincipal(lbp);
  try {
     art::ModuleDescription modDesc;
     modDesc.moduleName_ = "Blah";
     modDesc.moduleLabel_ = "blahs";
     art::Event event(ep, modDesc);

     std::string label("this does not exist");
     art::RefProd<edmtest::DummyProduct> ref = event.getRefBeforePut<edmtest::DummyProduct>(label);
     CPPUNIT_ASSERT("Failed to throw required exception" == 0);
  }
  catch (art::Exception& x) {
    // nothing to do
  }
  catch (...) {
    CPPUNIT_ASSERT("Threw wrong kind of exception" == 0);
  }

}

void testEventGetRefBeforePut::getRefTest() {
  std::string processName = "PROD";

  std::string label("fred");
  std::string productInstanceName("Rick");

  edmtest::IntProduct dp;
  art::TypeID dummytype(dp);
  std::string className = dummytype.friendlyClassName();

  art::ModuleDescription modDesc;
  modDesc.moduleName_ = "Blah";
  modDesc.parameterSetID_ = art::ParameterSet().id();

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
  art::EventID col(1L, 1L);
  std::string uuid = art::createGlobalIdentifier();
  art::Timestamp fakeTime;
  art::ProcessConfiguration pc(processName, art::ParameterSetID(), art::getReleaseVersion(), art::getPassID());
  boost::shared_ptr<art::ProductRegistry const> pregc(preg);
  art::RunAuxiliary runAux(col.run(), fakeTime, fakeTime);
  boost::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, pregc, pc));
  art::SubRunAuxiliary subRunAux(rp->run(), 1, fakeTime, fakeTime);
  boost::shared_ptr<art::SubRunPrincipal>lbp(new art::SubRunPrincipal(subRunAux, pregc, pc));
  lbp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(col, uuid, fakeTime, lbp->subRun(), true);
  art::EventPrincipal ep(eventAux, pregc, pc);
  ep.setSubRunPrincipal(lbp);

  art::RefProd<edmtest::IntProduct> refToProd;
  try {
    art::ModuleDescription modDesc;
    modDesc.moduleName_="Blah";
    modDesc.moduleLabel_=label;
    modDesc.processConfiguration_ = pc;

    art::Event event(ep, modDesc);
    std::auto_ptr<edmtest::IntProduct> pr(new edmtest::IntProduct);
    pr->value = 10;

    refToProd = event.getRefBeforePut<edmtest::IntProduct>(productInstanceName);
    event.put(pr,productInstanceName);
    event.commit_();
  }
  catch (artZ::Exception& x) {
    std::cerr << x.explainSelf()<< std::endl;
    CPPUNIT_ASSERT("Threw exception unexpectedly" == 0);
  }
  catch(std::exception& x){
     std::cerr <<x.what()<<std::endl;
     CPPUNIT_ASSERT("threw std::exception"==0);
  }
  catch (...) {
    std::cerr << "Unknown exception type\n";
    CPPUNIT_ASSERT("Threw exception unexpectedly" == 0);
  }
  CPPUNIT_ASSERT(refToProd->value == 10);
}

