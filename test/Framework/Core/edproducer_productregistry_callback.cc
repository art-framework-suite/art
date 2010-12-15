/**
   \file
   test for ProductRegistry

   \author Stefano ARGIRO
   \date 21 July 2005
*/


#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"

#include "art/Framework/Core/SignallingProductRegistry.h"
#include "art/Framework/Core/ConstProductRegistry.h"

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Actions.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Framework/Core/WorkerMaker.h"
#include "art/Framework/Core/WorkerParams.h"
#include "art/Framework/Core/WorkerT.h"

#include "fhiclcpp/ParameterSet.h"

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Utilities/TypeID.h"


class testEDProducerProductRegistryCallback: public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(testEDProducerProductRegistryCallback);

   CPPUNIT_TEST_EXCEPTION(testCircularRef,cet::exception);
   CPPUNIT_TEST_EXCEPTION(testCircularRef2,cet::exception);
   CPPUNIT_TEST(testTwoListeners);

CPPUNIT_TEST_SUITE_END();

public:
  void setUp(){}
  void tearDown(){}
  void testCircularRef();
  void testCircularRef2();
  void testTwoListeners();

};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testEDProducerProductRegistryCallback);

using namespace art;
using fhicl::ParameterSet;

namespace {
   class TestMod : public EDProducer
   {
     public:
      explicit TestMod(ParameterSet const& p);

      void produce(Event&);

      void listen(BranchDescription const&);
   };

   TestMod::TestMod(ParameterSet const&)
   { produces<int>();}

   void TestMod::produce(Event&)
   {
   }

   class ListenMod : public EDProducer
   {
     public:
      explicit ListenMod(ParameterSet const&);
      void produce(Event&);
      void listen(BranchDescription const&);
   };

   ListenMod::ListenMod(ParameterSet const&)
   {
      callWhenNewProductsRegistered(this,&ListenMod::listen);
   }
   void ListenMod::produce(Event&)
   {
   }

   void ListenMod::listen(BranchDescription const& iDesc)
   {
      art::TypeID intType(typeid(int));
      //std::cout << "see class " << iDesc.typeName() << std::endl;
      if(iDesc.friendlyClassName() == intType.friendlyClassName()) {
         produces<int>(iDesc.moduleLabel() + "-" + iDesc.productInstanceName());
         //std::cout << iDesc.moduleLabel() << "-" << iDesc.productInstanceName() << std::endl;
      }
   }

   class ListenFloatMod : public EDProducer
   {
public:
      explicit ListenFloatMod(ParameterSet const&);
      void produce(Event& e);
      void listen(BranchDescription const&);
   };

   ListenFloatMod::ListenFloatMod(ParameterSet const&)
   {
      callWhenNewProductsRegistered(this,&ListenFloatMod::listen);
   }
   void ListenFloatMod::produce(Event&)
   {
   }

   void ListenFloatMod::listen(BranchDescription const& iDesc)
   {
      art::TypeID intType(typeid(int));
      //std::cout <<"see class "<<iDesc.typeName()<<std::endl;
      if(iDesc.friendlyClassName() == intType.friendlyClassName()) {
         produces<float>(iDesc.moduleLabel()+"-"+iDesc.productInstanceName());
         //std::cout <<iDesc.moduleLabel()<<"-"<<iDesc.productInstanceName()<<std::endl;
      }
   }

}

void  testEDProducerProductRegistryCallback::testCircularRef(){
   using namespace art;

   SignallingProductRegistry preg;

   //Need access to the ConstProductRegistry service
   std::auto_ptr<ConstProductRegistry> cReg(new ConstProductRegistry(preg));
   ServiceToken token = ServiceRegistry::createContaining(cReg);
   ServiceRegistry::Operate startServices(token);

   std::auto_ptr<Maker> f(new WorkerMaker<TestMod>);

   ParameterSet p1;
   p1.put("@module_type",std::string("TestMod") );
   p1.put("@module_label",std::string("t1") );

   ParameterSet p2;
   p2.put("@module_type",std::string("TestMod") );
   p2.put("@module_label",std::string("t2") );

   art::ActionTable table;

   art::WorkerParams params1(p1, p1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams params2(p2, p2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());


   std::auto_ptr<Maker> lM(new WorkerMaker<ListenMod>);
   ParameterSet l1;
   l1.put("@module_type",std::string("ListenMod") );
   l1.put("@module_label",std::string("l1") );

   ParameterSet l2;
   l2.put("@module_type",std::string("ListenMod") );
   l2.put("@module_label",std::string("l2") );

   art::WorkerParams paramsl1(l1, l1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams paramsl2(l2, l2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());

   sigc::signal<void, const ModuleDescription&> aSignal;

   std::auto_ptr<Worker> w1 = f->makeWorker(params1,aSignal,aSignal);
   std::auto_ptr<Worker> wl1 = lM->makeWorker(paramsl1,aSignal,aSignal);
   std::auto_ptr<Worker> wl2 = lM->makeWorker(paramsl2,aSignal,aSignal);
   std::auto_ptr<Worker> w2 = f->makeWorker(params2,aSignal,aSignal);

   //Should be 5 products
   // 1 from the module 't1'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1'
   //    1 from 'l2' in response to 't1'
   //       1 from 'l1' in response to 'l2'
   // 1 from the module 't2'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1'
   //    1 from 'l2' in response to 't2'
   //       1 from 'l1' in response to 'l2'
   //std::cout <<"# products "<<preg.size()<<std::endl;
   CPPUNIT_ASSERT(10 == preg.size());
}

void  testEDProducerProductRegistryCallback::testCircularRef2(){
   using namespace art;

   SignallingProductRegistry preg;

   //Need access to the ConstProductRegistry service
   std::auto_ptr<ConstProductRegistry> cReg(new ConstProductRegistry(preg));
   ServiceToken token = ServiceRegistry::createContaining(cReg);
   ServiceRegistry::Operate startServices(token);

   std::auto_ptr<Maker> f(new WorkerMaker<TestMod>);

   ParameterSet p1;
   p1.put("@module_type",std::string("TestMod") );
   p1.put("@module_label",std::string("t1") );

   ParameterSet p2;
   p2.put("@module_type",std::string("TestMod") );
   p2.put("@module_label",std::string("t2") );

   art::ActionTable table;

   art::WorkerParams params1(p1, p1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams params2(p2, p2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());


   std::auto_ptr<Maker> lM(new WorkerMaker<ListenMod>);
   ParameterSet l1;
   l1.put("@module_type",std::string("ListenMod") );
   l1.put("@module_label",std::string("l1") );

   ParameterSet l2;
   l2.put("@module_type",std::string("ListenMod") );
   l2.put("@module_label",std::string("l2") );

   art::WorkerParams paramsl1(l1, l1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams paramsl2(l2, l2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());


   sigc::signal<void, const ModuleDescription&> aSignal;
   std::auto_ptr<Worker> wl1 = lM->makeWorker(paramsl1,aSignal,aSignal);
   std::auto_ptr<Worker> wl2 = lM->makeWorker(paramsl2,aSignal,aSignal);
   std::auto_ptr<Worker> w1 = f->makeWorker(params1,aSignal,aSignal);
   std::auto_ptr<Worker> w2 = f->makeWorker(params2,aSignal,aSignal);

   //Would be 10 products
   // 1 from the module 't1'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1' <-- circular
   //    1 from 'l2' in response to 't1'                  |
   //       1 from 'l1' in response to 'l2' <-- circular /
   // 1 from the module 't2'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1'
   //    1 from 'l2' in response to 't2'
   //       1 from 'l1' in response to 'l2'
   //std::cout <<"# products "<<preg.size()<<std::endl;
   CPPUNIT_ASSERT(10 == preg.size());
}

void  testEDProducerProductRegistryCallback::testTwoListeners(){
   using namespace art;

   SignallingProductRegistry preg;

   //Need access to the ConstProductRegistry service
   std::auto_ptr<ConstProductRegistry> cReg(new ConstProductRegistry(preg));
   ServiceToken token = ServiceRegistry::createContaining(cReg);
   ServiceRegistry::Operate startServices(token);

   std::auto_ptr<Maker> f(new WorkerMaker<TestMod>);

   ParameterSet p1;
   p1.put("@module_type",std::string("TestMod") );
   p1.put("@module_label",std::string("t1") );

   ParameterSet p2;
   p2.put("@module_type",std::string("TestMod") );
   p2.put("@module_label",std::string("t2") );

   art::ActionTable table;

   art::WorkerParams params1(p1, p1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams params2(p2, p2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());


   std::auto_ptr<Maker> lM(new WorkerMaker<ListenMod>);
   ParameterSet l1;
   l1.put("@module_type",std::string("ListenMod") );
   l1.put("@module_label",std::string("l1") );

   std::auto_ptr<Maker> lFM(new WorkerMaker<ListenFloatMod>);
   ParameterSet l2;
   l2.put("@module_type",std::string("ListenMod") );
   l2.put("@module_label",std::string("l2") );

   art::WorkerParams paramsl1(l1, l1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
   art::WorkerParams paramsl2(l2, l2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());


   sigc::signal<void, const ModuleDescription&> aSignal;
   std::auto_ptr<Worker> w1 = f->makeWorker(params1,aSignal,aSignal);
   std::auto_ptr<Worker> wl1 = lM->makeWorker(paramsl1,aSignal,aSignal);
   std::auto_ptr<Worker> wl2 = lFM->makeWorker(paramsl2,aSignal,aSignal);
   std::auto_ptr<Worker> w2 = f->makeWorker(params2,aSignal,aSignal);

   //Should be 8 products
   // 1 from the module 't1'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1'
   //    1 from 'l2' in response to 't1'
   // 1 from the module 't2'
   //    1 from 'l1' in response
   //       1 from 'l2' in response to 'l1'
   //    1 from 'l2' in response to 't2'
   //std::cout <<"# products "<<preg.size()<<std::endl;
   CPPUNIT_ASSERT(8 == preg.size());
}
