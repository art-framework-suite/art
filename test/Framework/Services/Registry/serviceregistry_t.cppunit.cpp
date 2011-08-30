/*
 *  serviceregistry_t.cppunit.cc
 *  CMSSW
 *
 *  Created by Chris Jones on 9/7/05.
 *
 */

//need to open a 'back door' to be able to setup the ServiceRegistry
#define private public
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#undef private
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "FWCore/ServiceRegistry/test/stubs/DummyService.h"

#include <cppunit/extensions/HelperMacros.h>

#include "art/Framework/PluginManager/ProblemTracker.h"

#include "boost/thread/thread.hpp"

class testServiceRegistry: public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(testServiceRegistry);

   CPPUNIT_TEST(loadTest);
   CPPUNIT_TEST(hierarchyTest);
   CPPUNIT_TEST(threadTest);
   CPPUNIT_TEST(externalServiceTest);

   CPPUNIT_TEST_SUITE_END();
public:
      void setUp(){}
   void tearDown(){}

   void loadTest();
   void hierarchyTest();
   void threadTest();
   void externalServiceTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testServiceRegistry);

void
testServiceRegistry::loadTest()
{
   art::AssertHandler ah;

   std::vector<fhicl::ParameterSet> pss;

   fhicl::ParameterSet ps;
   std::string typeName("DummyService");
   ps.addParameter("service_type", typeName);
   int value = 2;
   ps.addParameter("value", value);
   pss.push_back(ps);

   art::ServiceToken token(art::ServiceRegistry::createSet(pss));

   art::ServiceRegistry::Operate operate(token);
   art::ServiceHandle<testserviceregistry::DummyService> dummy;
   CPPUNIT_ASSERT(dummy);
   CPPUNIT_ASSERT(dummy.isAvailable());
   CPPUNIT_ASSERT(dummy->value() == 2);
}

namespace {
   struct DummyService { int value_; };
}

void
testServiceRegistry::externalServiceTest()
{
   art::AssertHandler ah;

   {
      std::auto_ptr<DummyService> dummyPtr(new DummyService);
      dummyPtr->value_ = 2;
      art::ServiceToken token(art::ServiceRegistry::createContaining(dummyPtr));
      {
         art::ServiceRegistry::Operate operate(token);
         art::ServiceHandle<DummyService> dummy;
         CPPUNIT_ASSERT(dummy);
         CPPUNIT_ASSERT(dummy.isAvailable());
         CPPUNIT_ASSERT(dummy->value_ == 2);
      }
      {
         std::vector<fhicl::ParameterSet> pss;

         fhicl::ParameterSet ps;
         std::string typeName("DummyService");
         ps.addParameter("service_type", typeName);
         int value = 2;
         ps.addParameter("value", value);
         pss.push_back(ps);

         art::ServiceToken token(art::ServiceRegistry::createSet(pss));
         art::ServiceToken token2(art::ServiceRegistry::createContaining(dummyPtr,
                                                                         token,
                                                                         art::serviceregistry::kOverlapIsError));

         art::ServiceRegistry::Operate operate(token2);
         art::ServiceHandle<testserviceregistry::DummyService> dummy;
         CPPUNIT_ASSERT(dummy);
         CPPUNIT_ASSERT(dummy.isAvailable());
         CPPUNIT_ASSERT(dummy->value() == 2);
      }
   }

   {
      std::auto_ptr<DummyService> dummyPtr(new DummyService);
      std::shared_ptr<art::serviceregistry::ServiceWrapper<DummyService> >
          wrapper(new art::serviceregistry::ServiceWrapper<DummyService>(dummyPtr));
      art::ServiceToken token(art::ServiceRegistry::createContaining(wrapper));

      wrapper->get().value_ = 2;

      {
         art::ServiceRegistry::Operate operate(token);
         art::ServiceHandle<DummyService> dummy;
         CPPUNIT_ASSERT(dummy);
         CPPUNIT_ASSERT(dummy.isAvailable());
         CPPUNIT_ASSERT(dummy->value_ == 2);
      }
      {
         std::vector<fhicl::ParameterSet> pss;

         fhicl::ParameterSet ps;
         std::string typeName("DummyService");
         ps.addParameter("service_type", typeName);
         int value = 2;
         ps.addParameter("value", value);
         pss.push_back(ps);

         art::ServiceToken token(art::ServiceRegistry::createSet(pss));
         art::ServiceToken token2(art::ServiceRegistry::createContaining(dummyPtr,
                                                                         token,
                                                                         art::serviceregistry::kOverlapIsError));

         art::ServiceRegistry::Operate operate(token2);
         art::ServiceHandle<testserviceregistry::DummyService> dummy;
         CPPUNIT_ASSERT(dummy);
         CPPUNIT_ASSERT(dummy.isAvailable());
         CPPUNIT_ASSERT(dummy->value() == 2);
      }

   }
}

void
testServiceRegistry::hierarchyTest()
{
   art::AssertHandler ah;

   std::vector<fhicl::ParameterSet> pss;
   {
      fhicl::ParameterSet ps;
      std::string typeName("DummyService");
      ps.addParameter("service_type", typeName);
      int value = 1;
      ps.addParameter("value", value);
      pss.push_back(ps);
   }
   art::ServiceToken token1(art::ServiceRegistry::createSet(pss));

   pss.clear();
   {
      fhicl::ParameterSet ps;
      std::string typeName("DummyService");
      ps.addParameter("service_type", typeName);
      int value = 2;
      ps.addParameter("value", value);
      pss.push_back(ps);
   }
   art::ServiceToken token2(art::ServiceRegistry::createSet(pss));


   art::ServiceRegistry::Operate operate1(token1);
   {
      art::ServiceHandle<testserviceregistry::DummyService> dummy;
      CPPUNIT_ASSERT(dummy->value() == 1);
   }
   {
      art::ServiceRegistry::Operate operate2(token2);
      art::ServiceHandle<testserviceregistry::DummyService> dummy;
      CPPUNIT_ASSERT(dummy->value() == 2);
   }
   {
      art::ServiceHandle<testserviceregistry::DummyService> dummy;
      CPPUNIT_ASSERT(dummy->value() == 1);
   }
}

namespace {
   struct UniqueRegistry {

      UniqueRegistry(void* iReg) : otherRegistry_(iReg) {}

      void operator()(){
         isUnique_ = (otherRegistry_ != &(art::ServiceRegistry::instance()));
      }
      void* otherRegistry_;
      static bool isUnique_;
   };
   bool UniqueRegistry::isUnique_ = false;

   struct PassServices {
      PassServices(art::ServiceToken iToken,
                    bool& oSucceeded,
                    bool& oCaughtException) :
                    token_(iToken), success_(&oSucceeded), caught_(&oCaughtException)
   { *success_ = false; *caught_ = false; }

      void operator()() {
         try  {
            art::ServiceRegistry::Operate operate(token_);
            art::ServiceHandle<testserviceregistry::DummyService> dummy;
            *success_ = dummy->value()==1;
         } catch(...){
            *caught_=true;
         }
      }

      art::ServiceToken token_;
      bool* success_;
      bool* caught_;

   };
}


void
testServiceRegistry::threadTest()
{
   UniqueRegistry::isUnique_ = false;
   void* value = &(art::ServiceRegistry::instance());
   UniqueRegistry unique(value);
   boost::thread testUniqueness(unique);
   testUniqueness.join();
   CPPUNIT_ASSERT(UniqueRegistry::isUnique_);



   art::AssertHandler ah;

   std::vector<fhicl::ParameterSet> pss;
   {
      fhicl::ParameterSet ps;
      std::string typeName("DummyService");
      ps.addParameter("service_type", typeName);
      int value = 1;
      ps.addParameter("value", value);
      pss.push_back(ps);
   }
   art::ServiceToken token(art::ServiceRegistry::createSet(pss));

   bool succeededToPassServices = false;
   bool exceptionWasThrown = false;

   PassServices passRun(token, succeededToPassServices, exceptionWasThrown);
   boost::thread testPassing(passRun);
   testPassing.join();
   CPPUNIT_ASSERT(!exceptionWasThrown);
   CPPUNIT_ASSERT(succeededToPassServices);
}
#include "test/CppUnit_testdriver.icpp"
