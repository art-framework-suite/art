////////////////////////////////////////////////////////////////////////
// Class:       HMSubRunProdProducer
// Module Type: producer
// File:        HMSubRunProdProducer_module.cc
//
// Generated at Tue Apr 15 13:28:20 2014 by Christopher Green using artmod
// from cetpkgsupport v1_05_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/Integration/high-memory/HMLargeData.h"

#include <memory>

namespace arttest {
  class HMSubRunProdProducer;
}

class arttest::HMSubRunProdProducer : public art::EDProducer {
public:
  explicit HMSubRunProdProducer(fhicl::ParameterSet const &);
  void produce(art::Event &) override { };
  void endSubRun(art::SubRun & sr) override;
};


arttest::HMSubRunProdProducer::HMSubRunProdProducer(fhicl::ParameterSet const &)
{
  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    produces<HMLargeData, art::InSubRun>(std::string("block") + std::to_string(i));
  }
}

void arttest::HMSubRunProdProducer::endSubRun(art::SubRun & sr)
{
  auto stencil = std::make_unique<HMLargeData>();
  std::iota(stencil->data(), stencil->data() + stencil->size(), 0);

  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    auto prod = std::make_unique<HMLargeData>(*stencil);
    sr.put(std::move(prod), std::string("block") + std::to_string(i), sr.seenRangeSet());
  }
}

DEFINE_ART_MODULE(arttest::HMSubRunProdProducer)
