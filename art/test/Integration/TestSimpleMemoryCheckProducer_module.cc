//--------------------------------------------------------------------
//
// SimpleMemoryCheckProducer module
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <vector>

namespace arttest {

  class TestSimpleMemoryCheckProducer : public art::EDProducer {
  public:
    explicit TestSimpleMemoryCheckProducer(fhicl::ParameterSet const&) {}

    void produce(art::Event&) override;

  private:
    std::vector<std::vector<int>> int_ptr_vec_;
  }; // TestSimpleMemoryCheckProducer

  void
  TestSimpleMemoryCheckProducer::produce(art::Event&)
  {
    int_ptr_vec_.emplace_back(100000, 0.);
  }
}

DEFINE_ART_MODULE(arttest::TestSimpleMemoryCheckProducer)
