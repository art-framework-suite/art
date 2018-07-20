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
#include "art/test/Integration/high-memory/HMLargeData.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using namespace std::string_literals;

namespace arttest {
  class HMSubRunProdProducer;
}

class arttest::HMSubRunProdProducer : public art::EDProducer {
public:
  explicit HMSubRunProdProducer(fhicl::ParameterSet const&);

private:
  void produce(art::Event&) override{};
  void endSubRun(art::SubRun& sr) override;
};

arttest::HMSubRunProdProducer::HMSubRunProdProducer(
  fhicl::ParameterSet const& ps)
  : EDProducer{ps}
{
  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    produces<HMLargeData, art::InSubRun>("block"s + std::to_string(i));
  }
}

void
arttest::HMSubRunProdProducer::endSubRun(art::SubRun& sr)
{
  auto stencil = std::make_unique<HMLargeData>();
  std::iota(stencil->data(), stencil->data() + stencil->size(), 0);

  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    auto prod = std::make_unique<HMLargeData>(*stencil);
    sr.put(
      std::move(prod), "block"s + std::to_string(i), art::subRunFragment());
  }
}

DEFINE_ART_MODULE(arttest::HMSubRunProdProducer)
