/**
   \file
   test for ProductRegistry

   \author Stefano ARGIRO
   \date 21 July 2005
*/


#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Framework/Core/EventProcessor.h"
// #include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "art/Framework/Core/SignallingProductRegistry.h"
#include "art/Framework/Services/System/ConstProductRegistry.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Framework/PluginManager/ProblemTracker.h"
#include "fhiclcpp/ParameterSet.h"

// namespace art {
//   class EDProduct;
// }

class testProductRegistry: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testProductRegistry);

CPPUNIT_TEST(testSignal);
CPPUNIT_TEST(testWatch);
   CPPUNIT_TEST_EXCEPTION(testCircular,cet::exception);

CPPUNIT_TEST(testProductRegistration);

CPPUNIT_TEST_SUITE_END();

public:
  testProductRegistry();
  void setUp();
  void tearDown();
  void testSignal();
  void testWatch();
  void testCircular();
  void testProductRegistration();

 private:
  art::ModuleDescription* intModule_;
  art::ModuleDescription* floatModule_;
  art::BranchDescription* intBranch_;
  art::BranchDescription* floatBranch_;
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testProductRegistry);

namespace {
   struct Listener {
      int* heard_;
      Listener(int& hear) :heard_(&hear) {}
      void operator()(const art::BranchDescription&){
         ++(*heard_);
      }
   };

   struct Responder {
      std::string name_;
      art::ProductRegistry* reg_;
      Responder(const std::string& iName,
                art::ConstProductRegistry& iConstReg,
                art::ProductRegistry& iReg):name_(iName),reg_(&iReg)
      {
        iConstReg.watchProductAdditions(this, &Responder::respond);
      }
      void respond(const art::BranchDescription& iDesc){
         art::ModuleDescription module;
	 module.parameterSetID_ = iDesc.parameterSetID();
         art::BranchDescription prod(iDesc.branchType(),
				     name_,
				     iDesc.processName(),
				     iDesc.fullClassName(),
				     iDesc.friendlyClassName(),
				     iDesc.productInstanceName() + "-" + name_,
				     module
				    );
         reg_->addProduct(prod);
      }
   };
}

testProductRegistry::testProductRegistry() :
  intModule_(0),
  floatModule_(0),
  intBranch_(0),
  floatBranch_(0)
{ }


void testProductRegistry::setUp()
{
  intModule_ = new art::ModuleDescription;
  intModule_->parameterSetID_ = fhicl::ParameterSet().id();
  intBranch_ = new art::BranchDescription(art::InEvent, "label", "PROD",
					  "int", "int", "int",
					  *intModule_);

  floatModule_ = new art::ModuleDescription;
  floatModule_->parameterSetID_ = intModule_->parameterSetID_;
  floatBranch_ = new art::BranchDescription(art::InEvent, "label", "PROD",
					    "float", "float", "float",
					    *floatModule_);

}

namespace
{
  template <class T> void kill_and_clear(T*& p) { delete p; p=0; }
}

void testProductRegistry::tearDown()
{
  kill_and_clear(floatBranch_);
  kill_and_clear(intBranch_);
  kill_and_clear(floatModule_);
  kill_and_clear(intModule_);
}

void  testProductRegistry:: testSignal(){
   using namespace art;
   SignallingProductRegistry reg;

   int hear=0;
   Listener listening(hear);
   reg.productAddedSignal_.connect(listening);

   //BranchDescription prod(InEvent, "label", "PROD", "int", "int", "int", md);

   //   reg.addProduct(prod);
   reg.addProduct(*intBranch_);
   CPPUNIT_ASSERT(1==hear);
}

void  testProductRegistry:: testWatch(){
   using namespace art;
   SignallingProductRegistry reg;
   ConstProductRegistry constReg(reg);

   int hear=0;
   Listener listening(hear);
   constReg.watchProductAdditions(listening);
   constReg.watchProductAdditions(listening, &Listener::operator());

   Responder one("one",constReg, reg);

   //BranchDescription prod(InEvent, "label", "PROD", "int", "int", "int");
   //reg.addProduct(prod);
   reg.addProduct(*intBranch_);

   //BranchDescription prod2(InEvent, "label", "PROD", "float", "float", "float");
   //   reg.addProduct(prod2);
   reg.addProduct(*floatBranch_);

   //Should be 4 products
   // 1 from the 'int' in this routine
   // 1 from 'one' responding to this call
   // 1 from the 'float'
   // 1 from 'one' responding to the original call
   CPPUNIT_ASSERT(4*2==hear);
   CPPUNIT_ASSERT(4 == reg.size());
}
void  testProductRegistry:: testCircular(){
   using namespace art;
   SignallingProductRegistry reg;
   ConstProductRegistry constReg(reg);

   int hear=0;
   Listener listening(hear);
   constReg.watchProductAdditions(listening);
   constReg.watchProductAdditions(listening, &Listener::operator());

   Responder one("one",constReg, reg);
   Responder two("two",constReg, reg);

   //BranchDescription prod(InEvent, "label","PROD","int","int","int");
   //reg.addProduct(prod);
   reg.addProduct(*intBranch_);

   //Should be 5 products
   // 1 from the original 'add' in this routine
   // 1 from 'one' responding to this call
   // 1 from 'two' responding to 'one'
   // 1 from 'two' responding to the original call
   // 1 from 'one' responding to 'two'
   CPPUNIT_ASSERT(5*2==hear);
   CPPUNIT_ASSERT(5 == reg.size());
}

void  testProductRegistry:: testProductRegistration(){
   art::AssertHandler ah;

  std::string configuration(
      "import FWCore.ParameterSet.Config as cms\n"
      "process = cms.Process('TEST')\n"
      "process.maxEvents = cms.untracked.PSet(\n"
      "  input = cms.untracked.int32(-1))\n"
      "process.source = cms.Source('DummySource')\n"
      "process.m1 = cms.EDProducer('TestPRegisterModule1')\n"
      "process.m2 = cms.EDProducer('TestPRegisterModule2')\n"
      "process.p = cms.Path(process.m1*process.m2)\n");
  try {
    art::EventProcessor proc(configuration, true);
  } catch(const cet::exception& iException) {
    std::cout <<"caught "<<iException.explain_self()<<std::endl;
    throw;
  }
}
