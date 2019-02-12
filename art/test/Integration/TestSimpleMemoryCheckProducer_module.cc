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
    struct Config {};
    using Parameters = art::EDProducer::Table<Config>;
    explicit TestSimpleMemoryCheckProducer(Parameters const&) {}

  private:
    void produce(art::Event&) override;
    std::vector<std::vector<int>> int_ptr_vec_;
  };

  void
  TestSimpleMemoryCheckProducer::produce(art::Event&)
  {
    int_ptr_vec_.emplace_back(100000, 0.);
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestSimpleMemoryCheckProducer)
