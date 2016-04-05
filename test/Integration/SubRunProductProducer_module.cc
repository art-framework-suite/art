//--------------------------------------------------------------------
//
// Main motivation for SubRunProductProducer is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"

#include "test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

namespace {

  class SubRunProductProducer : public art::EDProducer {

    template <typename T>
    auto producesInSubRun(std::string const& instance = {})
    {
      return produces<T,art::InSubRun>(instance);
    }

    unsigned events_ {};
    std::vector<int> eventNos_ {};

  public:

    explicit SubRunProductProducer(fhicl::ParameterSet const&)
    {
      produces<int>("eventNo");

      producesInSubRun<unsigned>("totalEvents");
      producesInSubRun<std::vector<int>>("eventNos");
      producesInSubRun<std::string>("subRunName");
    }

    void produce(art::Event& e) override
    {
      ++events_;
      eventNos_.emplace_back(e.event());
      e.put(std::make_unique<int>(eventNos_.back()), "eventNo");
    }

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<unsigned>(events_), "totalEvents");
      sr.put(std::make_unique<std::vector<int>>(eventNos_), "eventNos");
      sr.put(std::make_unique<std::string>("SubRun number: "+std::to_string(sr.subRun())), "subRunName");
      events_ = 0u;
      eventNos_.clear();
    }

  };  // SubRunProductProducer

}

DEFINE_ART_MODULE(SubRunProductProducer)
