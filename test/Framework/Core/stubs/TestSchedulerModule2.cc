/**
   Test Module for testScheduler
*/

static const char CVSId[] = "";

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <string>

namespace art{

  class TestSchedulerModule2 : public EDProducer
  {
  public:
    explicit TestSchedulerModule2(ParameterSet const& p):pset_(p){
       produces<arttest::StringProduct>();
    }

    void produce(Event& e, EventSetup const&);

  private:
    ParameterSet pset_;
  };


  void TestSchedulerModule2::produce(Event& e, EventSetup const&)
  {

    std::string myname = pset_.get<std::string>("module_name");
    std::unique_ptr<arttest::StringProduct> product(new arttest::StringProduct(myname));
    e.put(std::move(product));

  }
}//namespace

using art::TestSchedulerModule2;
DEFINE_ART_MODULE(TestSchedulerModule2)

// Local Variables:
// mode:c++
// End:
