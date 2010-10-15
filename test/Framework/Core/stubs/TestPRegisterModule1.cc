/**
   \file
   Test Modules for testProductRegistry

   \author Stefano ARGIRO
   \version
   \date 19 May 2005
*/


#include "art/Framework/Core/Event.h"

#include "test/TestObjects/ToyProducts.h"
#include "FWCore/Framework/test/stubs/TestPRegisterModule1.h"
#include <memory>
#include <string>

using namespace art;

static const char CVSId[] = "";

TestPRegisterModule1::TestPRegisterModule1(art::ParameterSet const& p):pset_(p){
   produces<arttest::StringProduct>();
}

void TestPRegisterModule1::produce(Event& e, EventSetup const&)
{

  std::string myname = pset_.getParameter<std::string>("@module_label");
  std::auto_ptr<arttest::StringProduct> product(new arttest::StringProduct(myname));
  e.put(product);
}
