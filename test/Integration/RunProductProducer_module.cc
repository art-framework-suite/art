//--------------------------------------------------------------------
//
// Main motivation for RunProductProducer is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"

#include "test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

namespace {

  class RunProductProducer : public art::EDProducer {

    template <typename T>
    auto producesInRun(std::string const& instance = {})
    {
      return produces<T,art::InRun>(instance);
    }

    unsigned events_ {};
    std::vector<int> eventNos_ {};

  public:

    explicit RunProductProducer(fhicl::ParameterSet const&)
    {
      produces<int>("eventNo");

      producesInRun<unsigned>("totalEvents");
      producesInRun<std::vector<int>>("eventNos");
      producesInRun<std::string>("runName");
    }

    void produce(art::Event& e) override
    {
      ++events_;
      eventNos_.emplace_back(e.event());
      e.put(std::make_unique<int>(eventNos_.back()), "eventNo");
    }

    void endRun(art::Run& r) override
    {
      r.put(std::make_unique<unsigned>(events_), "totalEvents");
      r.put(std::make_unique<std::vector<int>>(eventNos_), "eventNos");
      r.put(std::make_unique<std::string>("Run number: "+std::to_string(r.run())), "runName");
      events_ = 0u;
      eventNos_.clear();
    }

  };  // RunProductProducer

}

DEFINE_ART_MODULE(RunProductProducer)
