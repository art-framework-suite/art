/**
   \file
   Test Modules for ScheduleBuilder

   \author Stefano ARGIRO
   \version
   \date 19 May 2005
*/


#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"

#include "art/ParameterSet/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <memory>
#include <string>

using namespace art;

static const char CVSId[] = "";

class TestSchedulerModule1 : public EDProducer
{
 public:
  explicit TestSchedulerModule1(ParameterSet const& p):pset_(p){
    produces<arttest::StringProduct>();
  }

  void produce(Event& e, EventSetup const&);

private:
  ParameterSet pset_;
};


void TestSchedulerModule1::produce(Event& e, EventSetup const&)
{
  std::string myname = pset_.getParameter<std::string>("module_name");
  std::auto_ptr<arttest::StringProduct> product(new arttest::StringProduct(myname));
  e.put(product);
}

DEFINE_FWK_MODULE(TestSchedulerModule1);


// Configure (x)emacs for this file ...
// Local Variables:
// mode:c++
// compile-command: "make -C .. -k"
// End:
