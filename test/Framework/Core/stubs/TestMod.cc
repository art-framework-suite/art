
#include <iostream>

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"

#include "fhiclcpp/ParameterSet.h"

using namespace art;

class TestMod : public EDProducer
{
 public:
  explicit TestMod(ParameterSet const& p);

  void produce(Event& e, EventSetup const&);
};

TestMod::TestMod(ParameterSet const& p)
{
  produces<int>(); // We don't really produce anything.
  std::cerr << "TestMod instance created: " << p.get<std::string>("module_label")
            << std::endl;
}

void TestMod::produce(Event&, EventSetup const&)
{
  std::cerr << "Hi" << std::endl;
}

DEFINE_ART_MODULE(TestMod)
