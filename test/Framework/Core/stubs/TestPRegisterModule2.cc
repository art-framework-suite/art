/**
   \file
   Test Module for testProductRegistry

   \author Stefano ARGIRO
   \version
   \date 19 May 2005
*/

static const char CVSId[] = "";


#include "art/Framework/Core/Event.h"
#include "art/Persistency/Common/Handle.h"

#include "FWCore/Framework/test/stubs/TestPRegisterModule2.h"
#include "test/TestObjects/ToyProducts.h"
#include <cppunit/extensions/HelperMacros.h>
#include <memory>
#include <string>

using namespace art;

TestPRegisterModule2::TestPRegisterModule2(art::ParameterSet const&){
   produces<arttest::DoubleProduct>();
}

  void TestPRegisterModule2::produce(Event& e, EventSetup const&)
  {
     std::vector<art::Provenance const*> plist;
     e.getAllProvenance(plist);

     std::vector<art::Provenance const*>::const_iterator pd = plist.begin();

     CPPUNIT_ASSERT(0 !=plist.size());
     CPPUNIT_ASSERT(2 ==plist.size());
     CPPUNIT_ASSERT(pd != plist.end());
     arttest::StringProduct stringprod;
     art::TypeID stringID(stringprod);
     CPPUNIT_ASSERT(stringID.friendlyClassName() ==
                    (*pd)->friendlyClassName());
     CPPUNIT_ASSERT((*pd)->moduleLabel()=="m1");

     ++pd;
     CPPUNIT_ASSERT(pd != plist.end());

     arttest::DoubleProduct dprod;
     art::TypeID dID(dprod);
     CPPUNIT_ASSERT(dID.friendlyClassName() ==
                    (*pd)->friendlyClassName());
     CPPUNIT_ASSERT((*pd)->moduleLabel()=="m2");

    Handle<arttest::StringProduct> stringp;
    e.getByLabel("m2",stringp);
    CPPUNIT_ASSERT(stringp->name_=="m1");

     std::auto_ptr<arttest::DoubleProduct> product(new arttest::DoubleProduct);
     e.put(product);
  }
