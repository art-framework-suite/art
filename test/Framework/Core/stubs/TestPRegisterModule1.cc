/**
   \file
   Test Modules for testProductRegistry
*/

#include "FWCore/Framework/test/stubs/TestPRegisterModule1.h"
#include "art/Framework/Principal/Event.h"
#include "cpp0x/memory"
#include "test/TestObjects/ToyProducts.h"
#include <string>

using namespace art;

static const char CVSId[] = "";

TestPRegisterModule1::TestPRegisterModule1(fhicl::ParameterSet const& p):pset_(p){
   produces<arttest::StringProduct>();
}

void TestPRegisterModule1::produce(Event& e, EventSetup const&)
{

  std::string myname = pset_.get<std::string>("module_label");
  std::unique_ptr<arttest::StringProduct> product(new arttest::StringProduct(myname));
  e.put(std::move(product));
}
